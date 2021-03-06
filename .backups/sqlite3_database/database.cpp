#include "database.hpp"
#include "common.hpp"

#include <cstddef>
#include <sstream>
#include <iostream> //XXX

// TODO: Compress these statements? + FIX SELECT
static const char * const SELECT = 
  "SELECT "
    "$SELECT_COLUMNS "
  "FROM ("
    "SELECT DISTINCT "
      "t.url"            " AS url,"
      "t.album_url"      " AS album_url,"
      "t.title"          " AS title,"
      "t.artist"         " AS artist,"
      "t.remix"          " AS remix,"
      "t.number"         " AS number,"
      "t.bpm"            " AS bpm,"

      "a.artist"         " AS album_artist,"
      "a.title"          " AS album,"
      "a.cover_url"      " AS cover_url,"
      "a.description"    " AS description,"
      "a.date"           " AS date,"
      "a.rating"         " AS rating,"
      "a.votes"          " AS votes,"
      "a.download_count" " AS download_count,"

      "strftime('%Y',date) AS year,"
      "strftime('%m',date) AS month,"
      "strftime('%d',date) AS day,"

      "a_s.style"       " AS style,"

      "("
        "SELECT GROUP_CONCAT(style) "
        "FROM  album_style "
        "WHERE album_style.album_id = t.album_id"
      ") AS styles "
    "FROM "
      "track AS t "
    "JOIN album       AS a   ON a.id = t.album_id "
    "JOIN album_style AS a_s ON a.id = a_s.album_id "
  ")"

  "WHERE 1 $WHERE"
  "GROUP BY $GROUP_BY "
  "ORDER BY $ORDER_BY "
  "$LIMIT";

static const char * const CREATE_TABLES =
  "CREATE TABLE IF NOT EXISTS album"
    "(id"                " INTEGER PRIMARY KEY" // RowID, AutoIncrement
    ",url"               " TEXT NOT NULL UNIQUE"
    ",title"             " TEXT"
    ",artist"            " TEXT"
    ",cover_url"         " TEXT"
    ",description"       " TEXT"
    ",date"              " DATE"
    ",rating"            " FLOAT"
    ",votes"             " INT"
    ",download_count"    " INT"
    ");"

  "CREATE TABLE IF NOT EXISTS track"
    "(id"              " INTEGER PRIMARY KEY" // RowID, AutoIncrement
    ",url"             " TEXT NOT NULL UNIQUE"
    ",album_id"        " INTEGER NOT NULL REFERENCES album(id)"
    ",title"           " TEXT"
    ",artist"          " TEXT"
    ",remix"           " TEXT"
    ",number"          " INT"
    ",bpm"             " INT"
    ");"

  "CREATE TABLE IF NOT EXISTS style"
    "(id"            " INTEGER PRIMARY KEY" // RowID, AutoIncrement
    ",url"           " TEXT NOT NULL UNIQUE"
    ",name"          " TEXT"
    ");"

  "CREATE TABLE IF NOT EXISTS archive_url"
    "(album_id"        " INTEGER NOT NULL REFERENCES album(id)"
    ",url"             " TEXT NOT NULL UNIQUE"
    ",PRIMARY KEY (album_id, url));"

  "CREATE TABLE IF NOT EXISTS album_style"
    "(album_id"      " INTEGER NOT NULL REFERENCES album(id)"
    ",style_id"      " INTEGER NOT NULL REFERENCES style(id)"
    ",PRIMARY KEY (album_id, style_id))";

static const char * const SELECT_DESCRIPTION = 
  "SELECT description "
  "FROM   album "
  "WHERE  url = ?";

static const char * const SELECT_ARCHIVES = 
  "SELECT archive_url, archive_type "
  "FROM   archive_urls "
  "JOIN   track AS t ON t.album_url = archive_urls.album_url "
  "WHERE  t.url = ?";

void Database :: create_tables() {
  sqlite3_exec(db, CREATE_TABLES, NULL, NULL, NULL);
}

void Database :: begin_transaction() {
  sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
  // TODO: LOG ERR / throw
}

void Database :: end_transaction() {
  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
  // TODO: LOG ERR / throw
}

Database :: Database(const char *database_file) {
  sqlite3_open(database_file, &db); // TODO: ERR / throw
  sqlite3_extended_result_codes(db, 1);
  sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
  sqlite3_exec(db, "PRAGMA journal_mode = OFF", NULL, NULL, NULL);
  create_tables();
}

Database :: ~Database() {
  sqlite3_close(db);
}

  /*
void Database :: insert(const std::string &table, const std::map<std::string, std::string> &hash, const char* mode) {
  std::stringstream sql;
  sql << mode << " INTO " << table << '(';

  std::string columns;
  std::string placeholders;
  for (const auto p : hash) {
    columns      += p.first;
    columns      += ',';
    placeholders += "?,";
  }
  columns.resize(columns.size() - 1);
  placeholders.resize(placeholders.size() - 1);

  sql << columns << ") VALUES (" << placeholders << ")";

  std::cout << sql.str() << std::endl;

  sqlite3_stmt *stmt;
  prepare_v2(sql.str().c_str(), -1, &stmt, NULL);
  unsigned int i = 0;
  for (const auto &p : hash)
    bind_text(stmt, ++i, p.second.c_str(), -1);

  if (sqlite3_step(stmt) != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(db));

  //const char *s = (const char*) sqlite3_column_text(stmt, 0);
  //std::string ret = (s ? s : "");
  sqlite3_finalize(stmt);
  //return ret;
}
  */

unsigned int Database :: track_count() {
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM track", -1, &stmt, NULL);
  sqlite3_step(stmt);
  int ret = sqlite3_column_int(stmt, 0);
  sqlite3_finalize(stmt);
  return ret;
}

unsigned int Database :: album_count() {
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM album", -1, &stmt, NULL);
  sqlite3_step(stmt);
  int ret = sqlite3_column_int(stmt, 0);
  sqlite3_finalize(stmt);
  return ret;
}

std::string Database :: get_description(const char *album_url) {
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db, SELECT_DESCRIPTION, -1, &stmt, NULL);
  sqlite3_bind_text(stmt, 1, album_url, -1, NULL);
  sqlite3_step(stmt);
  const char *s = (const char*) sqlite3_column_text(stmt, 0);
  std::string ret = (s ? s : "");
  sqlite3_finalize(stmt);
  return ret;
}

void Database :: select
 (
  const std::string &columns,
  const std::vector<DatabaseFilter> &filters,
  const std::string &group_by,
  const std::string &order_by,
  int limit
 )
{
  std::string sql = SELECT;

  std::string where;
  for (const auto &filter : filters) {
    where += " AND ";
    where += filter.column;
    where += filter.op;
    where += '?';
  }

  std::string _limit;
  if (limit) {
    _limit = "LIMIT ";
    _limit += std::to_string(limit);
  }

  sql.replace(sql.find("$SELECT_COLUMNS"), STRLEN("$SELECT_COLUMNS"), columns);
  sql.replace(sql.find("$WHERE"),          STRLEN("$WHERE"),          where);
  sql.replace(sql.find("$GROUP_BY"),       STRLEN("$GROUP_BY"),       group_by);
  sql.replace(sql.find("$ORDER_BY"),       STRLEN("$ORDER_BY"),       order_by);
  sql.replace(sql.find("$LIMIT"),          STRLEN("$LIMIT"),          _limit);

  std::cout << sql << std::endl;

  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
  int i = 0;
  for (const auto &filter : filters) {
    sqlite3_bind_text(stmt, ++i, filter.value.c_str(), -1, NULL);
  }

  int ncolumns = sqlite3_column_count(stmt);

  for (;;) {
    int ret = sqlite3_step(stmt);
    if (ret == SQLITE_ROW) {
      std::cout << "having row" << std::endl;
      for (int i = 1; i < ncolumns; ++i) {
        const char *r = (const char*) sqlite3_column_text(stmt, i);
        if (!r)
          r = "";
        std::cout << r << std::endl;
      }
    }
    else {
      std::cout << "done: " << ret << std::endl;
      break;
    }
  }

  //const char *s = (const char*) sqlite3_column_text(stmt, 0);
  //std::string ret = (s ? s : "");
  sqlite3_finalize(stmt);
  //return ret;

  //sqlite_eecute(sql, where_params);
  // TODO: LOG ERR
}

#if TEST_DATABASE
#include <iostream>

int main() {
  Database db("/home/braph/.config/ektoplayer/meta.db");
  db.select("*", {}, "url", "album,number", 10);

  /*
  std::map<std::string,std::string> data = {{"style","TEST"}, {"url","PENIS"}};
  db.insert("styles", data);
  */

  std::cout << "Track count: " << db.track_count() << std::endl;
  std::cout << "Album count: " << db.album_count() << std::endl;

  /*
  printf("Found foo %s", db.get_description("sdfdsf").c_str());
  printf("Found foo %s", db.get_description("globular-entangled-everything").c_str());
  db.select();
  */

  return 0;
}
#endif
