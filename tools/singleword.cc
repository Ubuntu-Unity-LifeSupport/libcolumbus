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
 * A simple GUI application to search from a list of single words.
 */

#include "ColumbusHelpers.hh"
#include "LevenshteinIndex.hh"
#include "Word.hh"
#include "ErrorValues.hh"
#include "WordStore.hh"
#include <gtk/gtk.h>
#include <vector>
#include <cassert>
#include <cstring>
#include <cstdlib>

using namespace Columbus;
using namespace std;

const char *queryTime = "Query time: ";
const char *resultCount = "Total results: ";
const int DEFAULT_ERROR = 200;

struct app_data {
    LevenshteinIndex ind;
    WordStore store;
    ErrorValues e;
    GtkWidget *window;
    GtkWidget *entry;
    GtkListStore *matchStore;
    GtkWidget *matchView;
    GtkWidget *queryTimeLabel;
    GtkWidget *resultCountLabel;
    GtkWidget *errorSpinner;
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
    IndexMatches matches;
    GtkTreeIter iter;
    int maxError = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app->errorSpinner));
    double queryStart, queryEnd;
    try {
        Word query(gtk_entry_get_text(GTK_ENTRY(app->entry)));
        if(query.length() == 0)
            return;
        queryStart = hiresTimestamp();
        app->ind.findWords(query, app->e, maxError, matches);
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
        char buf[1024];
        app->store.getWord(matches.getMatch(i)).toUtf8(buf, 1024);
        gtk_list_store_append(app->matchStore, &iter);
        gtk_list_store_set(app->matchStore, &iter,
                0, buf,
                1, (int)matches.getMatchError(i),
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
    GtkTreeViewColumn *textColumn;
    GtkTreeViewColumn *errorColumn;
    app.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect (app.window, "delete-event",
            G_CALLBACK (delete_event), NULL);
    g_signal_connect (app.window, "destroy",
            G_CALLBACK (destroy), NULL);
    gtk_window_set_default_size(GTK_WINDOW(app.window), 600, 700);
    gtk_window_set_title(GTK_WINDOW(app.window), "Columbus single word search test tool");
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(app.window), vbox);
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    app.entry = gtk_entry_new();
    gtk_widget_set_tooltip_text(app.entry, "Word to search, must not contain whitespace.");
    app.errorSpinner = gtk_spin_button_new_with_range(100, 1000, 100);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(app.errorSpinner), DEFAULT_ERROR);
    gtk_widget_set_tooltip_text(app.errorSpinner, "Maximum error, 100 corresponds to one wrong letter.");
    g_signal_connect(app.entry, "changed", G_CALLBACK(doSearch), &app);
    g_signal_connect(app.errorSpinner, "value-changed", G_CALLBACK(doSearch), &app);

    app.matchStore = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    app.matchView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(app.matchStore));
    textColumn = gtk_tree_view_column_new_with_attributes("Match",
            gtk_cell_renderer_text_new(), "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app.matchView), textColumn);
    errorColumn = gtk_tree_view_column_new_with_attributes("Error",
            gtk_cell_renderer_text_new(), "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app.matchView), errorColumn);
    scroller = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroller), app.matchView);

    app.queryTimeLabel = gtk_label_new(queryTime);
    gtk_label_set_justify(GTK_LABEL(app.queryTimeLabel), GTK_JUSTIFY_LEFT);
    app.resultCountLabel = gtk_label_new(resultCount);
    gtk_label_set_justify(GTK_LABEL(app.resultCountLabel), GTK_JUSTIFY_LEFT);

    quitButton = gtk_button_new_with_label("Quit");
    g_signal_connect(quitButton, "clicked", G_CALLBACK(destroy), NULL);

    gtk_box_pack_start(GTK_BOX(hbox), app.entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), app.errorSpinner, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), scroller, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), app.queryTimeLabel, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), app.resultCountLabel, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), quitButton, FALSE, TRUE, 0);
    gtk_widget_show_all(app.window);
}

/*
 * Replace with library data read function once it is finished.
 */
static void readData(vector<Word> &a, const char *ifilename) {
    FILE *f = fopen(ifilename, "r");
    char buffer[1024];
    if(!f) {
        printf("Could not open dictionary file. Skipping performance test.\n");
        exit(0);
    }
    while(fgets(buffer, 1024, f) != NULL) {
        unsigned int slen = strlen(buffer);
        assert(buffer[slen-1] == '\n');
        buffer[slen-1] = '\0'; // Chop the \n.
        Word s(buffer);
        a.push_back(s);
    }
    fclose(f);
}

int main(int argc, char **argv) {
    app_data app;
    vector<Word> words;
    gtk_init(&argc, &argv);

    if(argc < 2) {
        printf("%s input_data_file.txt\n", argv[0]);
        return 0;
    }
    try {
        build_gui(app);
        readData(words, argv[1]);
        for(size_t i=0; i<words.size(); i++)
            app.ind.insertWord(words[i], app.store.getID(words[i]));
        gtk_main();
    } catch(std::exception &e) {
        fprintf(stderr, "Failed with exception: %s\n", e.what());
        return 99;
    }
    return 0;
}
