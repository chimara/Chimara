#include "babel/babel_handler.h"

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <libgda/libgda.h>
#include <sql-parser/gda-sql-parser.h>

typedef struct _metadata {
	const gchar *element_name;
	gchar *ifid;
	gchar *title;
	gchar *author;
	gchar *year;
} metadata;

void start_element(
	GMarkupParseContext *context,
	const gchar *element_name,
	const gchar **attribute_names,
	const gchar **attribute_values,
	gpointer data,
	GError **error)
{
	metadata *md = (metadata*) data;
	md->element_name = element_name;

	if( !strcmp(element_name, "ifindex") ) {
		md->ifid = g_strdup("");
		md->title = g_strdup("");
		md->author = g_strdup("");
		md->year = g_strdup("");
	}
}

void text(
	GMarkupParseContext *context,
	const gchar *text,
	gsize text_len,
	gpointer data,
	GError **error)
{
	metadata *md = (metadata*) data;
	gchar *stripped_text;

	if( !strcmp(md->element_name, "ifid") ) {
		stripped_text = g_strstrip( g_strndup(text, text_len) );
		md->ifid = g_strconcat(md->ifid, stripped_text, NULL);
		g_free(stripped_text);
	}
	else if( !strcmp(md->element_name, "title") ) {
		stripped_text = g_strstrip( g_strndup(text, text_len) );
		md->title = g_strconcat(md->title, stripped_text, NULL);
		g_free(stripped_text);
	}
	else if( !strcmp(md->element_name, "author") ) {
		stripped_text = g_strstrip( g_strndup(text, text_len) );
		md->author = g_strconcat(md->author, stripped_text, NULL);
		g_free(stripped_text);
	}
	else if( !strcmp(md->element_name, "firstpublished") ) {
		stripped_text = g_strstrip( g_strndup(text, text_len) );
		md->year = g_strconcat(md->year, stripped_text, NULL);
		g_free(stripped_text);
	}
}

void end_element(
	GMarkupParseContext *context,
	const gchar *element_name,
	gpointer data,
	GError **error)
{
	if( !strcmp(element_name, "ifindex") ) {
		metadata *md = (metadata*) data;
		printf("IFID: %s\nTitle: %s\nAuthor: %s\nYear: %s\n", md->ifid, md->title, md->author, md->year);
	}
}

/*
 * run a non SELECT command and stops if an error occurs
 */
void
run_sql_non_select(GdaConnection *cnc, const gchar *sql)
{
	GdaStatement *stmt;
	GError *error = NULL;
	gint nrows;
	const gchar *remain;
	GdaSqlParser *parser;

	parser = g_object_get_data(G_OBJECT(cnc), "parser");
	stmt = gda_sql_parser_parse_string(parser, sql, &remain, &error);
	if(remain) 
		g_print ("REMAINS: %s\n", remain);

	nrows = gda_connection_statement_execute_non_select(cnc, stmt, NULL, NULL, &error);
	if(nrows == -1)
		g_error("NON SELECT error: %s\n", error && error->message ? error->message : "no detail");
	g_object_unref(stmt);
}

int main(int argc, char **argv) {
	if(argc < 2) {
		fprintf(stderr, "Usage: %s <story file>\n", argv[0]);
		return 1;
	}

	babel_init(argv[1]);
	int len = babel_treaty(GET_STORY_FILE_METADATA_EXTENT_SEL, NULL, 0);
	if(len == 0) {
		printf("No metadata found.\n");
		babel_release();
		return 0;
	}

	gchar *buffer = malloc(len * sizeof(gchar));
	babel_treaty(GET_STORY_FILE_METADATA_SEL, buffer, len);
	g_strchomp(buffer);
	len = strlen(buffer);

	metadata data;
	GMarkupParser xml_parser = {start_element, end_element, text, NULL, NULL};
	GMarkupParseContext *context = g_markup_parse_context_new(&xml_parser, 0, &data, NULL);

	GError *err = NULL;
	if( g_markup_parse_context_parse(context, buffer, len, &err) == FALSE ) {
		fprintf(stderr, "Metadata parse failed: %s\n", err->message);
	}

	free(buffer);
	g_markup_parse_context_free(context);
	babel_release();

	// Open DB connection
	GdaConnection *cnc;
	GdaSqlParser *sql_parser;

	gda_init();
	cnc = gda_connection_open_from_string("SQLite", "DB_DIR=.;DB_NAME=library", NULL, GDA_CONNECTION_OPTIONS_NONE, &err);
	if(!cnc) {
		fprintf(stderr, "Could not open connection to SQLite database in library.db file: %s\n", err && err->message ? err->message : "No details");
		return 1;
	}

	sql_parser = gda_connection_create_parser(cnc);
	if(!sql_parser) // cnc does not provide its own parser, use default one
		sql_parser = gda_sql_parser_new();

	g_object_set_data_full(G_OBJECT(cnc), "parser", sql_parser, g_object_unref);
	
	// Create stories table
	run_sql_non_select(cnc, "DROP TABLE IF EXISTS stories");
	run_sql_non_select(cnc, "CREATE TABLE stories (ifid text not null primary key, title text, author text, year integer)");

	// Populate the table
	GValue *v1, *v2, *v3, *v4;
	v1 = gda_value_new_from_string(data.ifid, G_TYPE_STRING);
	v2 = gda_value_new_from_string(data.title, G_TYPE_STRING);
	v3 = gda_value_new_from_string(data.author, G_TYPE_STRING);
	v4 = gda_value_new_from_string(data.year, G_TYPE_UINT);

	if( !gda_insert_row_into_table(cnc, "stories", &err, "ifid", v1, "title", v2, "author", v3, "year", v4, NULL) ) {
		g_error("Could not INSERT data into the 'stories' table: %s\n", err && err->message ? err->message : "No details");
		return 1;
	}

	gda_value_free(v1);
	gda_value_free(v2);
	gda_value_free(v3);
	gda_value_free(v4);

	gda_connection_close(cnc);
	return 0;
}
