#include <gtk/gtk.h>
#ifdef  _WIN32
#include <windows.h>
#endif
#include <torch/torch.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <gtkmm-4.0/gtkmm.h>

torch::jit::script::Module module;

// GTK 控件
GtkWidget* result_label = nullptr;
GtkWidget* image_label = nullptr;
GdkPixbuf* pixbuf = nullptr;

bool load_model(const std::string &model_path)
{
    try
    {
        module = torch::jit::load(model_path);
        return true;
    } catch (const c10::Error &e)
    {
        std::cerr << "Model load failed: " << e.msg() << std::endl;
        return false;
    }
}

void proces_image_and_predict(const std::string& image_path)
{
    cv::Mat image = cv::imread(image_path);
    if (image.empty())
    {
        gtk_label_set_text(GTK_LABEL(result_label), "图像加载失败");
        return;
    }

    cv::Mat resized;
    cv::resize(image, resized, cv::Size(224, 224));

    // 归一化
    cv::Mat float_img;
    resized.convertTo(float_img, CV_32F, 1.0 / 255.0);


    torch::Tensor tensor_img = torch::from_blob(float_img.data, {1, 224, 224, 4}, torch::kFloat32);
    tensor_img = tensor_img.permute(({0, 3, 1, 2});  // HWC -> CHW

    // 推理
   std::vector<torch::jit::IValue> inputs;
   inputs.push_back(tensor_img);
   at::Tensor output = module.forward(inputs).toTensor();

   // 获取预测结果
   auto max_result = output.argmax(1);
   int predicted_class = max_result.item<int>();

   // 显示结果
   char buffer[100];
   sprintf(buffer, "预测类别: %d", predicted_class);
   gtk_label_set_text(GTK_LABEL(result_label), buffer);

   // 显示图像
   cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
   pixbuf = gdk_pixbuf_new_from_data(
       image.data,
       GDK_COLORSPACE_RGB,
       FALSE,
       8,
       image.cols,
       image.rows,
       image.step,
       NULL,
       NULL
   );

   GtkWidget *image_widget = gtk_image_new_from_pixbuf(pixbuf);
   gtk_container_add(GTK_CONTAINER(image_area), image_widget);
   gtk_widget_show_all(image_area);
}

static void on_image_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    dialog = gtk_file_chooser_dialog_new("选择图像", GTK_WINDOW(data),
                                         action,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         "_Open", GTK_RESPONSE_ACCEPT,
                                         NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        process_image_and_predict(std::string(filename));
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}


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
    if (!load_model("resnet18.pt")) {
        gtk_label_set_text(GTK_LABEL(result_label), "模型加载失败");
    }

    // 创建图像显示区域
    image_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(box), image_area, TRUE, TRUE, 0);

    // 创建结果标签
    result_label = gtk_label_new("等待图像");
    gtk_box_pack_start(GTK_BOX(box), result_label, TRUE, TRUE, 0);

    // 创建图像按钮
    GtkWidget *image_button = gtk_button_new_with_label("选择图像");
    g_signal_connect(image_button, "clicked", G_CALLBACK(on_image_button_clicked), window);
    gtk_box_pack_start(GTK_BOX(box), image_button, TRUE, TRUE, 0);

    // GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    // gtk_window_set_title(GTK_WINDOW(window), "GTK3 示例程序");
    // gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    // gtk_container_set_border_width(GTK_CONTAINER(window), 20);
    // g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    //
    // GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    // gtk_container_add(GTK_CONTAINER(window), box);
    //
    // GtkWidget *label = gtk_label_new("点击按钮显示当前时间");
    // gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);

    // GtkWidget *button = gtk_button_new_with_label("显示时间");
    // g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), label);
    // gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);

    // gtk_widget_show_all(window);
    // gtk_main();
    return 0;
}
