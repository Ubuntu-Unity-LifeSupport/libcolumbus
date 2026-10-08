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
 * A simple GUI application to test HUD matching.
 */

#include "columbus.hh"
#include "WordList.hh"
#include <gtk/gtk.h>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <cassert>

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
    vector<string> pathSource, commandSource;
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
                0, app->pathSource[id].c_str(),
                1, app->commandSource[id].c_str(),
                2, matches.getRelevancy(i),
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
    GtkWidget *scroller;
    GtkWidget *quitButton;
    GtkTreeViewColumn *pathColumn;
    GtkTreeViewColumn *commandColumn;
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
    app.entry = gtk_entry_new();
    gtk_widget_set_tooltip_text(app.entry, "You type your search phrase in, you blank your search phrase out. That's what it's all about.");
    g_signal_connect(app.entry, "changed", G_CALLBACK(doSearch), &app);

    app.matchStore = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_DOUBLE);
    app.matchView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(app.matchStore));
    pathColumn = gtk_tree_view_column_new_with_attributes("Path",
            gtk_cell_renderer_text_new(), "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app.matchView), pathColumn);
    commandColumn = gtk_tree_view_column_new_with_attributes("Command",
            gtk_cell_renderer_text_new(), "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app.matchView), commandColumn);
    relevancyColumn = gtk_tree_view_column_new_with_attributes("Relevancy",
            gtk_cell_renderer_text_new(), "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app.matchView), relevancyColumn);
    scroller = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroller), app.matchView);

    app.queryTimeLabel = gtk_label_new(queryTime);
    gtk_label_set_justify(GTK_LABEL(app.queryTimeLabel), GTK_JUSTIFY_LEFT);
    app.resultCountLabel = gtk_label_new(resultCount);
    gtk_label_set_justify(GTK_LABEL(app.resultCountLabel), GTK_JUSTIFY_LEFT);

    quitButton = gtk_button_new_with_label("Quit");
    g_signal_connect(quitButton, "clicked", G_CALLBACK(destroy), NULL);

    gtk_box_pack_start(GTK_BOX(vbox), app.entry, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), scroller, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), app.queryTimeLabel, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), app.resultCountLabel, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), quitButton, FALSE, TRUE, 0);
    gtk_widget_show_all(app.window);
}

void splitShowableParts(const string &line, string &pathText, string &commandText) {
    size_t tokenLoc = line.find('>', 0);
    pathText.assign(line, 0, tokenLoc);
    commandText.assign(line, tokenLoc+1, line.length());
}


void build_matcher(app_data &app, const char *dataFile) {
    Corpus *c = new Corpus();
    Word pathField("path");
    Word commandField("command");
    Word aliasField("alias");
    const size_t batchSize = 100000;
    size_t i=0;
    const double pathWeight = 0.3;
    const double aliasWeight = 0.8;
    double dataReadStart, dataReadEnd;

    ifstream ifile(dataFile);
    if(ifile.fail()) {
        printf("Could not open file %s.\n", dataFile);
        exit(1);
    }
    string line;

    app.m = new Matcher();
    app.m->getErrorValues().setSubstringMode();
    app.m->getErrorValues().addStandardErrors();
    // Build Corpus.
    dataReadStart = hiresTimestamp();
    while(getline(ifile, line)) {
        WordList path, command;
        string pathText, commandText;
        if(line.size() == 0)
            continue;
        // Remove possible DOS line ending garbage.
        if(line[line.size()-2] == '\r')
            line[line.size()-2] = '\0';
        splitShowableParts(line, pathText, commandText);
        path = splitToWords(pathText.c_str());
        command = splitToWords(commandText.c_str());
        if(command.size() == 0)
            continue;
        Document d(app.pathSource.size());
        d.addText(pathField, path);
        d.addText(commandField, command);
        if(commandText.find("Fuzzy Select") != (size_t)-1) {
            d.addText(aliasField, "magnetic lasso");
        }
        c->addDocument(d);
        app.pathSource.push_back(pathText);
        app.commandSource.push_back(commandText);
        i++;
        if(i % batchSize == 0) {
            app.m->index(*c);
            delete c;
            c = new Corpus();
        }
    }
    app.m->index(*c);
    delete c;
    app.m->getIndexWeights().setWeight(pathField, pathWeight);
    app.m->getIndexWeights().setWeight(aliasField, aliasWeight);
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
