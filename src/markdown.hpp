#ifndef MARKDOWN_HPP
#define MARKDOWN_HPP

#include <string>

namespace Html2Markdown {
  /**
   * Converts:
   *   Some HTML <a href="link url">link text</a> <b>bold</b> <i>italic</i>
   * to:
   *   Some HTML ((link text))[[url]] **bold** __italic__
   */
  std::string convert(const char*, size_t);
}

#endif
