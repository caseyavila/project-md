#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "scidown/src/html.h"
WebKitWebView *webView;

localization get_local() {
  localization local;
  local.figure = "Figure";
  local.listing = "Listing";
  local.table = "Table";
  return local;
}

enum renderer_type {
	RENDERER_HTML,
	RENDERER_LATEX,
	RENDERER_HTML_TOC
};

struct extension_category_info {
	unsigned int flags;
	const char *option_name;
	const char *label;
};

struct extension_info {
	unsigned int flag;
	const char *option_name;
	const char *description;
};

struct html_flag_info {
	unsigned int flag;
	const char *option_name;
	const char *description;
};

static struct extension_category_info categories_info[] = {
	{HOEDOWN_EXT_BLOCK, "block", "Block extensions"},
	{HOEDOWN_EXT_SPAN, "span", "Span extensions"},
	{HOEDOWN_EXT_FLAGS, "flags", "Other flags"},
	{HOEDOWN_EXT_NEGATIVE, "negative", "Negative flags"},
};

static struct extension_info extensions_info[] = {
	{HOEDOWN_EXT_TABLES, "tables", "Parse PHP-Markdown style tables."},
	{HOEDOWN_EXT_FENCED_CODE, "fenced-code", "Parse fenced code blocks."},
	{HOEDOWN_EXT_FOOTNOTES, "footnotes", "Parse footnotes."},

	{HOEDOWN_EXT_AUTOLINK, "autolink", "Automatically turn safe URLs into links."},
	{HOEDOWN_EXT_STRIKETHROUGH, "strikethrough", "Parse ~~stikethrough~~ spans."},
	{HOEDOWN_EXT_UNDERLINE, "underline", "Parse _underline_ instead of emphasis."},
	{HOEDOWN_EXT_HIGHLIGHT, "highlight", "Parse ==highlight== spans."},
	{HOEDOWN_EXT_QUOTE, "quote", "Render \"quotes\" as <q>quotes</q>."},
	{HOEDOWN_EXT_SUPERSCRIPT, "superscript", "Parse super^script."},
	{HOEDOWN_EXT_MATH, "math", "Parse TeX $$math$$ syntax, Kramdown style."},

	{HOEDOWN_EXT_NO_INTRA_EMPHASIS, "disable-intra-emphasis", "Disable emphasis_between_words."},
	{HOEDOWN_EXT_SPACE_HEADERS, "space-headers", "Require a space after '#' in headers."},
	{HOEDOWN_EXT_MATH_EXPLICIT, "math-explicit", "Instead of guessing by context, parse $inline math$ and $$always block math$$ (requires --math)."},
	{HOEDOWN_EXT_SCI, "scidown", "SciDown Extension"},
	{HOEDOWN_EXT_DISABLE_INDENTED_CODE, "disable-indented-code", "Don't parse indented code blocks."},
};

static struct html_flag_info html_flags_info[] = {
	{SCIDOWN_RENDER_SKIP_HTML, "skip-html", "Strip all HTML tags."},
	{SCIDOWN_RENDER_ESCAPE, "escape", "Escape all HTML."},
	{SCIDOWN_RENDER_HARD_WRAP, "hard-wrap", "Render each linebreak as <br>."},
	{SCIDOWN_RENDER_USE_XHTML, "xhtml", "Render XHTML."},
	{SCIDOWN_RENDER_MERMAID, "mermaid", "Render mermaid diagrams."},
	{SCIDOWN_RENDER_GNUPLOT, "gnuplot", "Render gnuplot plot."}
};

static const char *category_prefix = "all-";
static const char *negative_prefix = "no-";

#define DEF_IUNIT 1024
#define DEF_OUNIT 64
#define DEF_MAX_NESTING 16

static void dosomething (GtkTextBuffer *buffer, gpointer user_data) {
    GtkTextIter start_iter;
    GtkTextIter end_iter;

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

    webkit_web_view_load_html(webView, hoedown_buffer_cstr(html), "file:///home/casey/Proyectos/gtk/");

    hoedown_buffer_free(ib);
    hoedown_buffer_free(html);
    hoedown_document_free(document);
    hoedown_html_renderer_free(renderer);
}

static void activate (GtkApplication *app, gpointer user_data) {
    /* Declare variables */
    GtkWidget *window;
    GtkWidget *paned;
    GtkWidget *text_view;
    GtkWidget *scrolled_window;
    GtkTextBuffer *buffer;

    WebKitSettings *settings;

    paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);

    settings = webkit_settings_new();
    webkit_settings_set_allow_universal_access_from_file_urls(settings, TRUE);

    webView = WEBKIT_WEB_VIEW(webkit_web_view_new_with_settings(settings));

    /* Create a window with a title, and a default size */
    window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "project-md");
    //gtk_window_set_default_size (GTK_WINDOW(window), 220, 200);


    /* The text buffer represents the text being edited */
    buffer = gtk_text_buffer_new (NULL);


    /* Text view is a widget in which can display the text buffer. 
    * The line wrapping is set to break lines in between words.
    */
    text_view = gtk_text_view_new_with_buffer(buffer);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD); 
    gtk_text_view_set_monospace (GTK_TEXT_VIEW(text_view), TRUE); 
    g_signal_connect (buffer, "changed", G_CALLBACK(dosomething), NULL);


    gtk_paned_pack1(GTK_PANED(paned), GTK_WIDGET(text_view), TRUE, TRUE);
    gtk_paned_pack2(GTK_PANED(paned), GTK_WIDGET(webView), TRUE, TRUE);

    gtk_container_add(GTK_CONTAINER (window), GTK_WIDGET(paned));

    gtk_widget_show_all (window);
}

int main (int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK (activate), NULL);
    status = g_application_run(G_APPLICATION (app), argc, argv);
    g_object_unref(app);

    return status;
}

