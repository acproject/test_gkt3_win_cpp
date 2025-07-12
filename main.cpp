#include <ctime>     // 提供 time_t 类型
#include <cstdint>
#include <gtk/gtk.h>
 // 提供 intptr_t, uintptr_t 类型
// 按钮点击回调函数
static void on_button_clicked(GtkWidget *widget, gpointer data) {
    GtkLabel *label = (GtkLabel *)data;
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    gtk_label_set_text(label, time_str);
}

int main(int argc, char *argv[]) {
    // 初始化 GTK
    gtk_init(&argc, &argv);

    // 创建主窗口
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GTK3 示例程序");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    gtk_container_set_border_width(GTK_CONTAINER(window), 20);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // 创建垂直布局容器
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), box);

    // 创建标签
    GtkWidget *label = gtk_label_new("点击按钮显示当前时间");
    gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);

    // 创建按钮
    GtkWidget *button = gtk_button_new_with_label("显示时间");
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), label);
    gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);

    // 显示所有控件
    gtk_widget_show_all(window);

    // 进入 GTK 主循环
    gtk_main();

    return 0;
}