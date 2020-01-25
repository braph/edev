#ifndef _DATABASE_HPP
#define _DATABASE_HPP

#include <map>
#include <string>
#include <vector>
#include <sqlite3.h>

class DatabaseFilter {
  public:
    std::string column;
    std::string op;
    std::string value;

  DatabaseFilter(
    std::string _column,
    std::string _op,
    std::string _value
  ) :
    column(_column),
    op(_op),
    value(_value)
  { }
};

class Database {
  private:
    void create_tables();
    // drop_tables: %w(albums_styles archive_urls styles tracks albums).  each { |t| @db.execute("DROP TABLE IF EXISTS #{t}") }

  public:
    sqlite3 *db;
    unsigned int changed; // TODO

    Database(const char *database_file);
    ~Database();
    void begin_transaction();
    void end_transaction();

    /*
    inline void prepare_v2(const char *sql, int nByte, sqlite3_stmt **stmt, const char **pzTail) {
      int ret = sqlite3_prepare_v2(db, sql, nByte, stmt, pzTail);
      if (ret != SQLITE_OK)
        //throw std::runtime_error(sqlite3_errstr(ret));
        throw std::runtime_error(sqlite3_errmsg(db));
    }

    inline void bind_text(sqlite3_stmt *stmt, int col, const char *text, int n) {
      int ret = sqlite3_bind_text(stmt, col, text, n, NULL);
      if (ret != SQLITE_OK)
        throw std::runtime_error(sqlite3_errmsg(db));
    }
    */

    //void insert(const std::string &table, const std::map<std::string, std::string> &hash, const char* mode = "INSERT");
    unsigned int track_count();
    unsigned int album_count();
    std::string get_description(const char *album_url);
    void select
      (
       const std::string &columns = "number,artist,album,title,styles,date,year,rating,votes,download_count,bpm,album_url,url",
       const std::vector<DatabaseFilter> &filters = {},
       const std::string &group_by = "url",
       const std::string &order_by = "album,number",
       int limit = 0
      );
};

class Statement {
  private:
    sqlite3_stmt *m_stmt;

  public:
    inline Statement(Database *db, const char *sql) {
      sqlite3_prepare_v2(db->db, sql, -1, &m_stmt, NULL);
    }

    inline ~Statement() {
      sqlite3_finalize(m_stmt);
    }

    inline int reset() {
      return sqlite3_reset(m_stmt);
    }

    inline int clear_bindings() {
      return sqlite3_clear_bindings(m_stmt);
    }

    inline int bind(int n, const std::string &str) {
      return sqlite3_bind_text(m_stmt, n, str.c_str(), -1, NULL);
    }

    inline int bind(int n, int value) {
      return sqlite3_bind_int(m_stmt, n, value);
    }

    /*
    inline int bind(int n, short value) {
      return sqlite3_bind_int(m_stmt, n, value);
    }
    */

    inline int bind(int n, float value) {
      return sqlite3_bind_double(m_stmt, n, value);
    }

    inline int exec() {
      return sqlite3_step(m_stmt);
    }
};


#endif
