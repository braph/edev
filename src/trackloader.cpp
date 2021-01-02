#include "trackloader.hpp"

#include "ektoplayer.hpp"
#include "url_handler.hpp"
#include "config.hpp"

#include <lib/filesystem.hpp>

TrackLoader :: TrackLoader()
{
}

std::string TrackLoader :: get_file_for_track(Database::Tracks::Track track, bool force_download) {
  auto album_dir = Filesystem::path(Config::album_dir) / track.album().title();
  if (Filesystem::exists(album_dir)) {
    Filesystem::error_code e;
    auto track_no = track.number();
    for (const auto& f : Filesystem::directory_iterator(album_dir, e)) {
      if (std::atoi(f.path().filename().c_str()) == track_no) {
        log_write("Track %s -> ALBUM DIR: %s\n", track.title(), f.path());
        return f.path().string();
      }
    }
  }

  std::string track_url = track.url();
  Filesystem::path track_file = track_url;
  track_file += ".mp3";

  auto file_in_cache = Filesystem::path(Config::cache_dir) / track_file;

  if (force_download) {
    Filesystem::error_code e;
    Filesystem::remove(file_in_cache, e);
  }

  if (Filesystem::exists(file_in_cache)) {
    log_write("Track %s -> CACHE: %s\n", track.title(), file_in_cache);
    return file_in_cache.string();
  }

  Ektoplayer::url_expand(track_url, EKTOPLAZM_TRACK_BASE_URL, ".mp3");
  log_write("Track %s -> DOWNLOAD: %s\n", track.title(), track_url);

  FileDownload* download = new FileDownload(track_url, file_in_cache.string() +  ".part");
  download->setopt(CURLOPT_TIMEOUT, 60);

  download->onFinished = [=](Download& _dl, CURLcode e) {
    FileDownload& dl = static_cast<FileDownload&>(_dl);
    log_write("%s: %s [%d]\n", dl.last_url(), curl_easy_strerror(e), dl.http_code());

    Filesystem::error_code ex;
    if (e == CURLE_OK && dl.http_code() == 200)
      Filesystem::rename(dl.filename(), file_in_cache, ex);
    else
      Filesystem::remove(dl.filename(), ex);
  };

  _downloads.addDownload(download);
  return download->filename();
}

void TrackLoader :: download_album(const Database::Tracks::Track& track) {
  auto album_dir = Filesystem::path(Config::album_dir) / track.album().title();
  if (Filesystem::exists(album_dir))
    return;

  auto archive = Filesystem::path(Config::archive_dir) / track.album().title();
  if (Filesystem::exists(archive))
    return;

  std::string url = track.album().archive_mp3_url();
  Ektoplayer::url_expand(url, EKTOPLAZM_ARCHIVE_BASE_URL, "MP3.zip");
  log_write("Starting download: %s -> %s\n", url, archive.string() + ".part");

  FileDownload* download = new FileDownload(url, archive.string() +  ".part");
  download->onFinished = [=](Download& _dl, CURLcode e) {
    FileDownload& dl = static_cast<FileDownload&>(_dl);
    log_write("%s: %s [%d]\n", dl.last_url(), curl_easy_strerror(e), dl.http_code());

    Filesystem::error_code ex;
    if (e == CURLE_OK && dl.http_code() == 200) {
      if (Config::auto_extract_to_archive_dir) {
        unpack_archive(dl.filename().c_str(), album_dir.c_str());
        // TODO if (Config::delete_after_extraction) {}
      }
      else {
        Filesystem::rename(dl.filename(), archive, ex);
      }
    }
    else
      Filesystem::remove(dl.filename(), ex);
  };

  _downloads.addDownload(download);
}
