#include "babel/babel_handler.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <libgda/libgda.h>
#include <sql-parser/gda-sql-parser.h>
#include <ghttp.h>

typedef struct _metadata {
	const gchar *element_name;
	gchar *ifid;
	gchar *title;
	gchar *author;
	gchar *firstpublished;
	gboolean error;
	gchar *error_message;
	gchar *error_code;
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

	if( !strcmp(element_name, "errorCode") ) {
		md->error = 1;
		md->error_message = "";
		md->error_code = "";
	}

	if( !strcmp(element_name, "ifindex") ) {
		md->ifid = "";
		md->title = "";
		md->author = "";
		md->firstpublished = "";
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

	if( !strcmp(md->element_name, "errorCode") ) {
		md->error_code = g_strndup(text, text_len);
	}
	else if( !strcmp(md->element_name, "errorMessage") ) {
		md->error_message = g_strndup(text, text_len);
	}
	else if( !strcmp(md->element_name, "ifid") ) {
		if( strlen(md->ifid) < text_len )
			md->ifid = g_strndup(text, text_len);
	}
	else if( !strcmp(md->element_name, "title") ) {
		if( strlen(md->title) < text_len )
			md->title = g_strndup(text, text_len);
	}
	else if( !strcmp(md->element_name, "author") ) {
		if( strlen(md->author) < text_len )
			md->author = g_strndup(text, text_len);
	}
	else if( !strcmp(md->element_name, "firstpublished") ) {
		if( strlen(md->firstpublished) < text_len )
			md->firstpublished = g_strndup(text, text_len);
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
		printf("IFID: %s\nTitle: %s\nAuthor: %s\nFirst published: %s\n", md->ifid, md->title, md->author, md->firstpublished);
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
	GError *err = NULL;
	metadata data;
	data.error = 0;

	if(argc < 2) {
		fprintf(stderr, "Usage: %s <story file>\n", argv[0]);
		return 1;
	}

	babel_init(argv[1]);
	int len = babel_treaty(GET_STORY_FILE_METADATA_EXTENT_SEL, NULL, 0);
	gchar *ifiction;
	if(len) {
		printf("Metadata found in file.\n");
		gchar *buffer = malloc(len * sizeof(gchar));
		babel_treaty(GET_STORY_FILE_METADATA_SEL, buffer, len);
		ifiction = g_strndup(buffer, len);
		g_free(buffer);
	} else {
		printf("No metadata found in file, performing IFDB lookup.\n");
		gchar *ifid = malloc(TREATY_MINIMUM_EXTENT * sizeof(gchar));
		if( !babel_treaty(GET_STORY_FILE_IFID_SEL, ifid, TREATY_MINIMUM_EXTENT) ) {
			fprintf(stderr, "Unable to create an IFID (A serious problem occurred while loading the file).\n");
			babel_release();
			return 1;
		}
		printf("Looking up IFID: %s.\n", ifid);
		babel_release();

		ghttp_request *request = ghttp_request_new();
		ghttp_set_uri(request, g_strconcat("http://ifdb.tads.org/viewgame?ifiction&ifid=", ifid, NULL));
		ghttp_set_header(request, http_hdr_Connection, "close");
		ghttp_prepare(request);
		ghttp_process(request);

		ifiction = g_strndup( ghttp_get_body(request), ghttp_get_body_len(request) );
		ghttp_request_destroy(request);
	}


	ifiction = g_strchomp(ifiction);

	GMarkupParser xml_parser = {start_element, end_element, text, NULL, NULL};
	GMarkupParseContext *context = g_markup_parse_context_new(&xml_parser, 0, &data, NULL);

	if( g_markup_parse_context_parse(context, ifiction, strlen(ifiction), &err) == FALSE ) {
		fprintf(stderr, "Metadata parse failed: %s\n", err->message);
	}

	g_markup_parse_context_free(context);
	g_free(ifiction);

	babel_release();

	// Check for errors
	if(data.error) {
		fprintf(stderr, "ERROR %s: %s\n", data.error_code, data.error_message);
		return 1;
	}

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
	//run_sql_non_select(cnc, "DROP TABLE IF EXISTS stories");
	run_sql_non_select(cnc, "CREATE TABLE IF NOT EXISTS stories (ifid text not null primary key, title text, author text, firstpublished text)");

	// Populate the table
	GValue *v1, *v2, *v3, *v4;
	v1 = gda_value_new_from_string(data.ifid, G_TYPE_STRING);
	v2 = gda_value_new_from_string(data.title, G_TYPE_STRING);
	v3 = gda_value_new_from_string(data.author, G_TYPE_STRING);
	v4 = gda_value_new_from_string(data.firstpublished, G_TYPE_STRING);

	if( !gda_insert_row_into_table(cnc, "stories", &err, "ifid", v1, "title", v2, "author", v3, "firstpublished", v4, NULL) ) {
		g_warning("Could not INSERT data into the 'stories' table: %s\n", err && err->message ? err->message : "No details");
	}

	gda_value_free(v1);
	gda_value_free(v2);
	gda_value_free(v3);
	gda_value_free(v4);

	// Dump the table contents
	GdaDataModel *data_model;
	GdaStatement *stmt = gda_sql_parser_parse_string(sql_parser, "SELECT * FROM stories", NULL, NULL);
	data_model = gda_connection_statement_execute_select(cnc, stmt, NULL, &err);
	if(!data_model)
		g_error("Could not get the contents of the 'stories' table: %s\n", err && err->message ? err->message : "No details");
	printf("Dumping library table:\n");
	gda_data_model_dump(data_model, stdout);

	g_object_unref(stmt);
	g_object_unref(data_model);

	gda_connection_close(cnc);
	return 0;
}
