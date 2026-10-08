/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *    Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU Lesser General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * A simple GUI application to search the database.
 */

#include "columbus.hh" // This app should only need public API from Columbus.
#include <gtk/gtk.h>
#include <string>
#include <fstream>
#include <vector>

using namespace Columbus;
using namespace std;

const char *queryTime = "Query time: ";
const char *resultCount = "Total results: ";
const int DEFAULT_ERROR = 200;

struct app_data {
    Matcher *m;
    GtkWidget *window;
    GtkWidget *entry;
    GtkListStore *matchStore;
    GtkWidget *matchView;
    GtkWidget *queryTimeLabel;
    GtkWidget *resultCountLabel;
    vector<string> source;
};

static gboolean delete_event(GtkWidget */*widget*/, GdkEvent */*event*/, gpointer /*data*/) {
    gtk_main_quit();
    return TRUE;
}

static void destroy(GtkWidget */*widget*/, gpointer /*data*/) {
    gtk_main_quit ();
}

static void doSearch(GtkWidget */*widget*/, gpointer data) {
    app_data *app = (app_data*) data;
    MatchResults matches;
    GtkTreeIter iter;
    double queryStart, queryEnd;
    try {
        queryStart = hiresTimestamp();
        matches = app->m->match(gtk_entry_get_text(GTK_ENTRY(app->entry)));
        queryEnd = hiresTimestamp();
    } catch(exception &e) {
        printf("Matching failed: %s\n", e.what());
        gtk_list_store_clear(app->matchStore);
        gtk_label_set_text(GTK_LABEL(app->queryTimeLabel), queryTime);
        gtk_label_set_text(GTK_LABEL(app->resultCountLabel), resultCount);
        gtk_entry_set_text(GTK_ENTRY(app->entry), "");
        return;
    }
    gtk_list_store_clear(app->matchStore);
    for(size_t i=0; i<matches.size(); i++) {
        DocumentID id = matches.getDocumentID(i);
        gtk_list_store_append(app->matchStore, &iter);
        gtk_list_store_set(app->matchStore, &iter,
                0, app->source[id].c_str(),
                1, matches.getRelevancy(i),
                -1);
    }
    char buf[1024];
    sprintf(buf, "%s%.2f", queryTime, queryEnd - queryStart);
    gtk_label_set_text(GTK_LABEL(app->queryTimeLabel), buf);
    sprintf(buf, "%s%lu", resultCount, (unsigned long) matches.size());
    gtk_label_set_text(GTK_LABEL(app->resultCountLabel), buf);

}

void build_gui(app_data &app) {
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *scroller;
    GtkWidget *quitButton;
    GtkWidget *searchButton;
    GtkTreeViewColumn *textColumn;
    GtkTreeViewColumn *relevancyColumn;
    app.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect (app.window, "delete-event",
            G_CALLBACK (delete_event), NULL);
    g_signal_connect (app.window, "destroy",
            G_CALLBACK (destroy), NULL);
    gtk_window_set_default_size(GTK_WINDOW(app.window), 600, 700);
    gtk_window_set_title(GTK_WINDOW(app.window), "Columbus query tool");
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(app.window), vbox);
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    app.entry = gtk_entry_new();
    gtk_widget_set_tooltip_text(app.entry, "Word to search, must not contain whitespace.");
    searchButton = gtk_button_new_with_label("Search");
    g_signal_connect(searchButton, "clicked", G_CALLBACK(doSearch), &app);
    g_signal_connect(app.entry, "activate", G_CALLBACK(doSearch), &app); // GTK+ docs say not to connect to "activate" but it seems to work.

    app.matchStore = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_DOUBLE);
    app.matchView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(app.matchStore));
    textColumn = gtk_tree_view_column_new_with_attributes("Match",
            gtk_cell_renderer_text_new(), "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app.matchView), textColumn);
    relevancyColumn = gtk_tree_view_column_new_with_attributes("Relevancy",
            gtk_cell_renderer_text_new(), "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app.matchView), relevancyColumn);
    scroller = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroller), app.matchView);

    app.queryTimeLabel = gtk_label_new(queryTime);
    gtk_label_set_justify(GTK_LABEL(app.queryTimeLabel), GTK_JUSTIFY_LEFT);
    app.resultCountLabel = gtk_label_new(resultCount);
    gtk_label_set_justify(GTK_LABEL(app.resultCountLabel), GTK_JUSTIFY_LEFT);

    quitButton = gtk_button_new_with_label("Quit");
    g_signal_connect(quitButton, "clicked", G_CALLBACK(destroy), NULL);

    gtk_box_pack_start(GTK_BOX(hbox), app.entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), searchButton, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), scroller, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), app.queryTimeLabel, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), app.resultCountLabel, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), quitButton, FALSE, TRUE, 0);
    gtk_widget_show_all(app.window);
}

void build_matcher(app_data &app, const char *dataFile) {
    Corpus *c = new Corpus();
    Word field("name");
    const size_t batchSize = 100000;
    size_t i=0;
    double dataReadStart, dataReadEnd;

    ifstream ifile(dataFile);
    if(ifile.fail()) {
        printf("Could not open file %s.\n", dataFile);
        exit(1);
    }
    string line;

    app.m = new Matcher();
    // Build Corpus.
    dataReadStart = hiresTimestamp();
    while(getline(ifile, line)) {
        if(line.size() == 0)
            continue;
        // Remove possible DOS line ending garbage.
        if(line[line.size()-2] == '\r')
            line[line.size()-2] = '\0';
        Document d(app.source.size());
        d.addText(field, line.c_str());
        c->addDocument(d);
        app.source.push_back(line);
        i++;
        if(i % batchSize == 0) {
            app.m->index(*c);
            delete c;
            c = new Corpus();
        }
    }
    app.m->index(*c);
    delete c;
    dataReadEnd = hiresTimestamp();
    printf("Read in %lu documents in %.2f seconds.\n", (unsigned long) i, dataReadEnd - dataReadStart);
}

void delete_matcher(app_data &app) {
    delete app.m;
    app.m = 0;
}
int main(int argc, char **argv) {
    app_data app;
    double buildStart, buildEnd;
    gtk_init(&argc, &argv);

    if(argc < 2) {
        printf("%s input_data_file.txt\n", argv[0]);
        return 0;
    }
    try {
        build_gui(app);
        buildStart = hiresTimestamp();
        build_matcher(app, argv[1]);
        buildEnd = hiresTimestamp();
        printf("Building the matcher took %.2f seconds.\n", buildEnd - buildStart);
        gtk_main();
        delete_matcher(app);
    } catch(std::exception &e) {
        fprintf(stderr, "Failed with exception: %s\n", e.what());
        return 99;
    }
    return 0;
}
