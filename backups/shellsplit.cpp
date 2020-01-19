
  case '\'':
      for (;++it;)
        if (!*it)               throw std::invalid_argument("missing terminating single quote");
        else if (*it == '\'')   break;
        else                    value += *it;

      ++it;
      break;


        while (*++it)
          if (*it == '\'') { ++it; goto read_word; }
          else             { values += *it;        }
        throw std::invalid_argument("missing terminating single quote");

  case '"';
        while (*++it)
          if      (*it == '"')  { ++it; goto read_word; }
          else if (*it == '\\') { ++it; value += *it;   }
          else                  { 


        for (;++it;)
          if (!*it)               throw std::invalid_argument("missing terminating double quote");
          else if (*it == '"')    { break;        }
          else if (*it == '\\')   { ++it;         }
          else                    { value += *it; }





  for (const char *it = s.c_str(); *it; ++it) {

    if (escaped) {
      value += *it;
      escaped = 0;
      continue;
    }

    switch (*it) {
      case ' ':
      case '\t':
        if (in_word):
          goto add_word;
        break;
      case '"':
      case '\'':
        if (quote == *it)
          quote = '\0';
        else 
          quote = *it;
      case '\\':
        if (quote)
          ;
    }


    if (escaped || in_word)
      value += *c;
    else if (quote == '\'')


    switch (*it) {
      case '"':
        if (
        break;
      case '\'':
        break;
      case ' ':
      case '\t':
        break;
    }
