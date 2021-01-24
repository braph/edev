#include "trackloader.hpp"

#include "ektoplayer.hpp"
#include "programs.hpp"
#include "config.hpp"

#include <lib/filesystem.hpp>
#include <lib/process.hpp>

#include <unistd.h>

std::string TrackLoader :: get_file_for_track(Database::Tracks::Track track, bool force_download) {
  Filesystem::error_code e;

  auto album_dir = Filesystem::path(Config::album_dir) / track.album().title();
  if (Filesystem::exists(album_dir)) {
    auto track_no = track.number();
    for (const auto& f : Filesystem::directory_iterator(album_dir, e)) {
      if (std::atoi(f.path().filename().c_str()) == track_no) {
        log_write("Track %s -> ALBUM DIR: %s\n", track.title(), f);
        return f.path().string();
      }
    }
  }

  std::string track_url = track.url();
  Filesystem::path track_file = track_url;
  track_file += ".mp3";

  auto file_in_cache = Filesystem::path(Config::cache_dir) / track_file;

  if (force_download)
    Filesystem::remove(file_in_cache, e);

  if (Filesystem::exists(file_in_cache)) {
    log_write("Track %s -> CACHE: %s\n", track.title(), file_in_cache);
    return file_in_cache.string();
  }

  Ektoplayer::url_expand(track_url, EKTOPLAZM_TRACK_BASE_URL, ".mp3");
  log_write("Track %s -> DOWNLOAD: %s\n", track.title(), track_url);

  auto download = new FileDownload(track_url, file_in_cache.string() + EKTOPLAZM_DOWNLOAD_SUFFIX);
  download->setopt(CURLOPT_TIMEOUT, 60);

  _downloads.add_download(download, [=](Download& _dl, CURLcode e) {
    FileDownload& dl = static_cast<FileDownload&>(_dl);
    log_write("%s: %s [%d]\n", dl.effective_url(), curl_easy_strerror(e), dl.http_code());

    Filesystem::error_code ex;
    if (e == CURLE_OK && dl.http_code() == 200)
      Filesystem::rename(dl.filename(), file_in_cache, ex);
    else
      Filesystem::remove(dl.filename(), ex);

    return Downloads::Action::Remove;
  });

  return download->filename();
}

void TrackLoader :: download_album(const Database::Tracks::Track& track) {
  auto album_dir = Filesystem::absolute(Config::album_dir) / track.album().title();
  if (Filesystem::exists(album_dir))
    return;

  auto archive = Filesystem::absolute(Config::archive_dir) / track.album().title();
  if (Filesystem::exists(archive))
    return;

  std::string url = track.album().archive_mp3_url();
  Ektoplayer::url_expand(url, EKTOPLAZM_ARCHIVE_BASE_URL, "MP3.zip");

  auto download = new FileDownload(url, archive.string() +  EKTOPLAZM_DOWNLOAD_SUFFIX);
  log_write("Starting download: %s -> %s\n", url, download->filename());

  _downloads.add_download(download, [=](Download& dl_, CURLcode e) {
    auto& dl = static_cast<FileDownload&>(dl_);
    log_write("%s: %s [%d]\n", dl.effective_url(), curl_easy_strerror(e), dl.http_code());

    Filesystem::error_code ex;
    if (e == CURLE_OK && dl.http_code() == 200) {
      if (Config::auto_extract_to_archive_dir) {
        auto file     = std::move(dl.filename()); // dl will vanish
        auto dest_dir = std::move(album_dir);

        Process([file, dest_dir](){
          Filesystem::error_code e;
          if (Filesystem::create_directory(dest_dir, e)) {
            if (0 != Programs::file_archiver(file, dest_dir).get_exit_status())
              Filesystem::remove_all(dest_dir, e);
            else if (Config::delete_after_extraction)
              Filesystem::remove(file, e);
          }
        }).detach();
      }
      else {
        Filesystem::rename(dl.filename(), archive, ex);
      }
    }
    else
      Filesystem::remove(dl.filename(), ex);

    return Downloads::Action::Remove;
  });
}
