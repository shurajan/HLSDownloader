import hls_downloader

import os
# Запуск 4 воркеров
hls_downloader.start_workers(4)

hls_downloader.add_task("https://edge24-ams.live.mmcdn.com/live-hls/..1/playlist.m3u8", "output1.ts")
hls_downloader.add_task("https://edge16-hel.live.mmcdn.com/live-hls/..2/playlist.m3u8", "output2.ts")
hls_downloader.add_task("https://edge16-hel.live.mmcdn.com/live-hls/..3/playlist.m3u8", "output3.ts")
hls_downloader.add_task("https://edge16-hel.live.mmcdn.com/live-hls/..4/playlist.m3u8", "output4.ts")

hls_downloader.stop_downloads()

