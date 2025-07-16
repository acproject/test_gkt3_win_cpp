#include <gtk/gtk.h>
#ifdef  _WIN32
#include <windows.h>
#endif
#include <torch/torch.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
 // 提供 intptr_t, uintptr_t 类型
// 按钮点击回调函数
static void on_button_clicked(GtkWidget *widget, gpointer data) {
    auto label = static_cast<GtkLabel *>(data);
    auto now = time(NULL);
    auto* time_str = ctime(&now);
    gtk_label_set_text(label, time_str);
}
#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#else
int main(int argc, char **argv) {
#endif
    int argc = 0;
    char **argv = NULL;
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GTK3 示例程序");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    gtk_container_set_border_width(GTK_CONTAINER(window), 20);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), box);

    GtkWidget *label = gtk_label_new("点击按钮显示当前时间");
    gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);

    GtkWidget *button = gtk_button_new_with_label("显示时间");
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), label);
    gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
