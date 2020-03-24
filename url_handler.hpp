#ifndef URL_HANDLER_HPP
#define URL_HANDLER_HPP

#include <boost/algorithm/string/predicate.hpp>

#include <unistd.h>

#include <string>
#include <cstdlib>

static inline void open_image(const std::string& url) {
  if (fork() == 0) { // TODO: can be made better...
    execl("/bin/sh", "sh", "-c",
      "URL=$1; shift;"
      "for CMD; do :|$CMD \"$URL\" && break;"
      "done >/dev/null 2>/dev/null", 

      "open_image",
      // $URL
      url.c_str(),
      // $CMD's
      "feh",
      "display",
      "xdg-open",
      NULL);
    std::exit(0);
  }
}

static inline void open_url(const std::string& url) {
  if (boost::algorithm::iends_with(url, ".png")
      || boost::algorithm::iends_with(url, ".jpg")
      || boost::algorithm::iends_with(url, ".jpeg"))
    open_image(url);
  else
    if (fork() == 0) {
      execlp("xdg-open", "xdg-open", url.c_str(), NULL);
      std::exit(0);
    }
}

#endif
