#include <gtk/gtk.h>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <torch/torch.h>
#include <torch/script.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cstdio>
#include <ctime>
#include <iostream>
// AppData 结构体用于封装应用状态
typedef struct {
    GtkWidget *result_label;
    GtkWidget *image_area;
    GdkPixbuf *pixbuf;
    torch::jit::script::Module module;
} AppData;

bool load_model(AppData *data, const std::string &model_path) {
    try {
        data->module = torch::jit::load(model_path);
        return true;
    } catch (const c10::Error &e) {
        std::cerr << "Model load failed: " << e.msg() << std::endl;
        return false;
    }
}

void process_image_and_predict(AppData *data, const std::string& image_path) {
    cv::Mat image = cv::imread(image_path);
    if (image.empty()) {
        gtk_label_set_text(GTK_LABEL(data->result_label), "图像加载失败");
        return;
    }

    if (data->pixbuf) {
        g_object_unref(data->pixbuf);
        data->pixbuf = nullptr;
    }

    cv::Mat resized;
    cv::resize(image, resized, cv::Size(224, 224));

    cv::Mat float_img;
    resized.convertTo(float_img, CV_32F, 1.0 / 255.0);

    torch::Tensor tensor_img = torch::from_blob(float_img.data, {1, 224, 224, 3}, torch::kFloat32);
    tensor_img = tensor_img.permute({0, 3, 1, 2});

    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(tensor_img);
    at::Tensor output = data->module.forward(inputs).toTensor();

    auto max_result = output.argmax(1);
    int predicted_class = max_result.item<int>();

    char buffer[100];
    sprintf(buffer, "预测类别: %d", predicted_class);
    gtk_label_set_text(GTK_LABEL(data->result_label), buffer);

    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
    data->pixbuf = gdk_pixbuf_new_from_data(
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

    GtkWidget *image_widget = gtk_picture_new_for_pixbuf(data->pixbuf);
    gtk_box_append(GTK_BOX(data->image_area), image_widget);
}

static void on_open_response(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    AppData *data = (AppData *)user_data;
    GFile *file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(source_object), res, NULL);
    if (file) {
        char *filename = g_file_get_path(file);
        process_image_and_predict(data, std::string(filename));
        g_free(filename);
        g_object_unref(file);
    }
}

static void on_image_button_clicked(GtkWidget *widget, gpointer user_data) {
    AppData *data = (AppData *)user_data;
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_open(dialog, GTK_WINDOW(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), NULL, on_open_response, data);
    g_object_unref(dialog);
}

static void on_window_destroy(GtkWidget *widget, gpointer user_data) {
        AppData *data = (AppData *)user_data;
        if (data->pixbuf) {
            g_object_unref(data->pixbuf);
        }
        g_free(data);
}

static void activate(GtkApplication *app, gpointer user_data) {
    AppData *data = g_new(AppData, 1);
    if (!load_model(data, "resnet18.pth")) {
        std::cerr << "Failed to load model" << std::endl;
        return;
    }

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "GTK4 示例程序");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_window_set_child(GTK_WINDOW(window), box);

    data->image_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_append(GTK_BOX(box), data->image_area);

    data->result_label = gtk_label_new("等待图像");
    gtk_box_append(GTK_BOX(box), data->result_label);

    GtkWidget *image_button = gtk_button_new_with_label("选择图像");
    g_signal_connect(image_button, "clicked", G_CALLBACK(on_image_button_clicked), data);
    gtk_box_append(GTK_BOX(box), image_button);

    

    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), data);

    gtk_widget_set_visible(window, TRUE);
}
#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int argc = __argc;
    char **argv = __argv;
#else
int main(int argc, char **argv) {
#endif
    GtkApplication *app = gtk_application_new("org.example.app", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
