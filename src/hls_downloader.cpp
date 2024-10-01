#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <thread>
#include <vector>
#include <queue>
#include <iostream>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <csignal>
#include <spdlog/spdlog.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libavutil/error.h>
}

namespace py = pybind11;

std::atomic<bool> stop_flag(false);

// Очередь задач
std::queue<std::pair<std::string, std::string>> task_queue;
std::mutex queue_mutex;
std::condition_variable queue_cond_var;

// Обработчик сигнала Ctrl+C
void signalHandler(int signum) {
    spdlog::warn("Прерывание программы (Ctrl+C), завершаем загрузку...");
    stop_flag = true;
    queue_cond_var.notify_all();  // Разбудим все воркеры для завершения
}

void setupSignalHandler() {
    std::signal(SIGINT, signalHandler);
}

void download_hls_stream(const std::string& input_url, const std::string& output_filename) {
    AVFormatContext* input_format_ctx = nullptr;
    AVFormatContext* output_format_ctx = nullptr;

    // Логируем начало закачки
    spdlog::info("Начинаем закачку: {} -> {}", input_url, output_filename);

    // Открытие HLS-потока
    if (avformat_open_input(&input_format_ctx, input_url.c_str(), nullptr, nullptr) != 0) {
        spdlog::error("Не удалось открыть поток: {}", input_url);
        return;
    }

    // Найдем информацию о потоках
    if (avformat_find_stream_info(input_format_ctx, nullptr) < 0) {
        spdlog::error("Не удалось найти информацию о потоке: {}", input_url);
        avformat_close_input(&input_format_ctx);
        return;
    }

    // Создаем формат для выходного файла
    avformat_alloc_output_context2(&output_format_ctx, nullptr, "mpegts", output_filename.c_str());
    if (!output_format_ctx) {
        spdlog::error("Не удалось создать выходной формат для файла: {}", output_filename);
        avformat_close_input(&input_format_ctx);
        return;
    }

    // Копируем параметры потоков
    for (unsigned int i = 0; i < input_format_ctx->nb_streams; i++) {
        AVStream* in_stream = input_format_ctx->streams[i];
        AVStream* out_stream = avformat_new_stream(output_format_ctx, nullptr);
        if (!out_stream) {
            spdlog::error("Не удалось создать выходной поток");
            avformat_close_input(&input_format_ctx);
            avformat_free_context(output_format_ctx);
            return;
        }
        avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        out_stream->codecpar->codec_tag = 0;
    }

    // Открываем выходной файл
    if (!(output_format_ctx->flags & AVFMT_NOFILE)) {
        if (avio_open(&output_format_ctx->pb, output_filename.c_str(), AVIO_FLAG_WRITE) < 0) {
            spdlog::error("Не удалось открыть файл для записи: {}", output_filename);
            avformat_close_input(&input_format_ctx);
            avformat_free_context(output_format_ctx);
            return;
        }
    }

    // Записываем заголовки в выходной файл
    avformat_write_header(output_format_ctx, nullptr);

    // Чтение и запись пакетов
    AVPacket packet;
    while (!stop_flag && av_read_frame(input_format_ctx, &packet) >= 0) {
        AVStream* in_stream = input_format_ctx->streams[packet.stream_index];
        AVStream* out_stream = output_format_ctx->streams[packet.stream_index];

        packet.pts = av_rescale_q_rnd(packet.pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF);
        packet.dts = av_rescale_q_rnd(packet.dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF);
        packet.duration = av_rescale_q(packet.duration, in_stream->time_base, out_stream->time_base);
        packet.pos = -1;

        av_interleaved_write_frame(output_format_ctx, &packet);
        av_packet_unref(&packet);
    }

    av_write_trailer(output_format_ctx);
    avformat_close_input(&input_format_ctx);
    avio_closep(&output_format_ctx->pb);
    avformat_free_context(output_format_ctx);

    // Логируем завершение закачки
    spdlog::info("Завершена закачка: {} -> {}", input_url, output_filename);
}

// Функция для выполнения задач из очереди
void worker() {
    while (!stop_flag) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_cond_var.wait(lock, [] { return !task_queue.empty() || stop_flag; });

        if (!task_queue.empty()) {
            auto task = task_queue.front();
            task_queue.pop();
            lock.unlock();

            // Выполняем закачку
            download_hls_stream(task.first, task.second);
        } else {
            lock.unlock();
        }
    }
}

// Функция для добавления задачи на закачку
void add_task(const std::string& url, const std::string& output) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        task_queue.emplace(url, output);
        spdlog::info("Добавлена новая задача: {} -> {}", url, output);
    }
    queue_cond_var.notify_one();
}

// Функция для запуска воркеров
void start_workers(int num_workers) {
    setupSignalHandler();  // Устанавливаем обработчик сигнала Ctrl+C

    spdlog::info("Запуск {} воркеров", num_workers);

    for (int i = 0; i < num_workers; ++i) {
        std::thread(worker).detach();
    }
}

// Функция для остановки закачек
void stop_downloads() {
    stop_flag = true;
    queue_cond_var.notify_all();  // Разбудим все воркеры для завершения
    spdlog::info("Все закачки остановлены");
}

PYBIND11_MODULE(hls_downloader, m) {
    m.def("add_task", &add_task, "Добавить новую задачу на закачку",
          py::arg("url"), py::arg("output"));
    m.def("start_workers", &start_workers, "Запустить воркеров",
          py::arg("num_workers"));
    m.def("stop_downloads", &stop_downloads, "Остановить все закачки");
}
