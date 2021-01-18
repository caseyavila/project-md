#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "scidown/src/html.h"

#define DEF_IUNIT 1024
#define DEF_OUNIT 64
#define DEF_MAX_NESTING 16


localization get_local() {
    localization local;
    local.figure = "Figure";
    local.listing = "Listing";
    local.table = "Table";
    return local;
}

static void update_web_view(GtkTextBuffer *buffer, gpointer user_data) {
    GtkTextIter start_iter;
    GtkTextIter end_iter;
    WebKitWebView *web_view = WEBKIT_WEB_VIEW(user_data);

    ext_definition def = {NULL, NULL};
    def.extra_header = "<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/KaTeX/0.9.0-alpha2/katex.min.css\" crossorigin=\"anonymous\">\n"
                        "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/KaTeX/0.9.0-alpha2/katex.min.js\" crossorigin=\"anonymous\"></script>\n"
                        "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/KaTeX/0.9.0-alpha2/contrib/auto-render.min.js\" crossorigin=\"anonymous\"></script>\n";
    def.extra_closing = "<script>renderMathInElement(document.body);</script>\n";

    hoedown_buffer *html = hoedown_buffer_new(DEF_OUNIT);

    gtk_text_buffer_get_start_iter(buffer, &start_iter);
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    char *text = gtk_text_buffer_get_text(buffer, &start_iter, &end_iter, FALSE);

    hoedown_buffer *ib = hoedown_buffer_new(DEF_IUNIT);
    hoedown_buffer_puts(ib, text);

    hoedown_renderer *renderer = hoedown_html_renderer_new(SCIDOWN_RENDER_CHARTER, 0, get_local());

    hoedown_document *document = hoedown_document_new(renderer, HOEDOWN_EXT_BLOCK | HOEDOWN_EXT_SPAN | HOEDOWN_EXT_FLAGS, &def, NULL, DEF_MAX_NESTING);

    hoedown_document_render(document, html, hoedown_buffer_cstr(ib), ib->size, -1);

    webkit_web_view_load_html(web_view, hoedown_buffer_cstr(html), "file:///home/casey/Proyectos/project-md/");

    hoedown_buffer_free(ib);
    hoedown_buffer_free(html);
    hoedown_document_free(document);
    hoedown_html_renderer_free(renderer);
}

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    GtkWidget *window;
    GtkWidget *paned;
    GtkWidget *text_view;
    GtkTextBuffer *buffer;
    WebKitWebView *web_view;

    WebKitSettings *settings;

    paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);

    web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "project-md");

    buffer = gtk_text_buffer_new(NULL);

    text_view = gtk_text_view_new_with_buffer(buffer);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD); 
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(text_view), TRUE); 
    g_signal_connect(buffer, "changed", G_CALLBACK(update_web_view), web_view);


    gtk_paned_pack1(GTK_PANED(paned), GTK_WIDGET(text_view), FALSE, FALSE);
    gtk_paned_pack2(GTK_PANED(paned), GTK_WIDGET(web_view), FALSE, FALSE);

    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(paned));

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}

