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
#include <fstream>
#include <nlohmann/json.hpp>
#include <map>
// AppData 结构体用于封装应用状态
typedef struct {
    GtkWidget *result_label;
    GtkWidget *image_area;
    GdkPixbuf *pixbuf;
    torch::jit::script::Module module;
    bool model_loaded;
    cv::Mat persistent_image; // 新增，确保图像数据生命周期
} AppData;

bool load_model(AppData *data, const std::string &model_path) {
    std::cout << "[LOG] 开始加载model" << model_path << std::endl;
    try {
        data->module = torch::jit::load(model_path);
        if (torch::cuda::is_available()) {
            std::cout << "[LOG] CUDA可用，模型转移到GPU" << std::endl;
            data->module.to(torch::kCUDA);
        } else {
            std::cout << "[LOG] CUDA不可用，使用CPU" << std::endl;
        }
        std::cout << "[LOG] 模型加载成功" << std::endl;
        data->model_loaded = true;
        return true;
    } catch (const c10::Error &e) {
        std::string error_msg = "Model load failed: " + e.msg();
        std::cerr << error_msg << std::endl;
        std::ofstream log_file("load_error.log", std::ios::app);
        auto cout_buf = std::cout.rdbuf();
        std::cout.rdbuf(log_file.rdbuf()); // 重定向std::cout到文件

        // 这里是模型加载代码
        try {
            data->module = torch::jit::load(model_path);
            data->model_loaded = true;
            std::cout << "Model loaded successfully from " << model_path << std::endl;
        } catch (const c10::Error& e) {
            std::cout << "Model load failed: " << e.what() << std::endl;
        }

        std::cout.rdbuf(cout_buf); // 恢复std::cout
        log_file.close();
        if (log_file.is_open()) {
            log_file << error_msg << std::endl;
            log_file.close();
        }
        return false;
    }
}

// 全局类别标签映射
std::map<int, std::string> class_labels;

// 加载类别标签映射
void load_class_labels(const std::string& json_path) {
    std::ifstream in(json_path);
    nlohmann::json j;
    in >> j;
    class_labels.clear();
    for (auto& [key, value] : j.items()) {
        int idx = std::stoi(key);
        class_labels[idx] = value[1];
    }
}
void process_image_and_predict(AppData *data, const std::string& image_path) {
    std::ofstream log_file("app_log.log",std::ios::app);
    auto cout_buf = std::cout.rdbuf();
    std::cout.rdbuf(log_file.rdbuf());

    std::cout << "[LOG]开始处理图像：" << image_path << std::endl;
    if (!data->model_loaded) {
        std::cout << "[LOG]模型未加载:"  << std::endl;
        gtk_label_set_text(GTK_LABEL(data->result_label), "请先加载模型");
        std::cout.rdbuf(cout_buf);
        log_file.close();
        return;
    }
    cv::Mat image = cv::imread(image_path);
    if (image.empty()) {
        std::cout << "[ERROR] 图像加载失败: " << image_path << std::endl;
        gtk_label_set_text(GTK_LABEL(data->result_label), "图像加载失败");
        std::cout.rdbuf(cout_buf);
        log_file.close();
        return;
    }
    std::cout << "[LOG] 图像加载成功" << std::endl;

    if (data->pixbuf) {
        g_object_unref(data->pixbuf);
        data->pixbuf = nullptr;
    }

    cv::Mat resized;
    cv::resize(image, resized, cv::Size(224, 224));

    cv::Mat float_img;
    resized.convertTo(float_img, CV_32F, 1.0 / 255.0);

    std::cout << "[LOG] 开始创建Tensor" << std::endl;
    torch::Tensor tensor_img = torch::from_blob(float_img.data, {1, 224, 224, 3}, torch::kFloat32);
    tensor_img = tensor_img.permute({0, 3, 1, 2});
    std::cout << "[LOG] 张量创建成功" << std::endl;

    if (torch::cuda::is_available()) {
        tensor_img = tensor_img.to(torch::kCUDA);
    }

    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(tensor_img);
    std::cout << "[LOG] start predicting..." << std::endl;
    int predicted_class = -1;
    try {
        at::Tensor output = data->module.forward(inputs).toTensor();
        std::cout << "[LOG] 模型预测完成" << std::endl;

        auto max_result = output.argmax(1);
        predicted_class = max_result.item<int>();
        std::cout << "[LOG] 模型预测完成 " << std::endl;
        predicted_class = max_result.item<int>();
        std::cout << "[LOG] 模型预测类别: " << predicted_class << std::endl;
    } catch (const c10::Error& e) {
        std::cout << "[ERROR] 模型预测失败 " << e.what() << std::endl;
        gtk_label_set_text(GTK_LABEL(data->result_label), "预测失败");
        std::cout.rdbuf(cout_buf);
        log_file.close();
        return;
    }

    // 显示类别标签
    std::string label_str = "预测类别: " + std::to_string(predicted_class);
    if (!class_labels.empty() && class_labels.count(predicted_class)) {
        label_str += " (" + class_labels[predicted_class] + ")";
    }
    gtk_label_set_text(GTK_LABEL(data->result_label), label_str.c_str());

    std::cout.rdbuf(cout_buf);
    log_file.close();

    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
    // 修正：将image变量提升作用域，确保数据生命周期
    data->persistent_image = image.clone();
    data->pixbuf = gdk_pixbuf_new_from_data(
        data->persistent_image.data,
        GDK_COLORSPACE_RGB,
        FALSE,
        8,
        data->persistent_image.cols,
        data->persistent_image.rows,
        data->persistent_image.step,
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

static void on_load_model_clicked(GtkWidget *widget, gpointer user_data) {
    AppData *data = (AppData *)user_data;
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_open(dialog, GTK_WINDOW(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), NULL, +[](GObject *source_object, GAsyncResult *res, gpointer user_data) {
        AppData *data = (AppData *)user_data;
        GFile *file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(source_object), res, NULL);
        if (file) {
            char *filename = g_file_get_path(file);
            if (load_model(data, std::string(filename))) {
                gtk_label_set_text(GTK_LABEL(data->result_label), "模型加载成功");
            } else {
                gtk_label_set_text(GTK_LABEL(data->result_label), "模型加载失败");
            }
            g_free(filename);
            g_object_unref(file);
        }
    }, data);
    g_object_unref(dialog);
}

static void activate(GtkApplication *app, gpointer user_data) {
    std::cout << "[LOG] 进入 activate 回调" << std::endl;
    AppData *data = g_new0(AppData, 1);
    data->model_loaded = false;
    // 启动时加载类别标签
    load_class_labels("imagenet_class_index.json");
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "GTK4 示例程序");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 800);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20); 
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_window_set_child(GTK_WINDOW(window), box);
    data->image_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_append(GTK_BOX(box), data->image_area);
    data->result_label = gtk_label_new("等待操作");
    gtk_box_append(GTK_BOX(box), data->result_label);
    GtkWidget *load_model_button = gtk_button_new_with_label("加载模型");
    g_signal_connect(load_model_button, "clicked", G_CALLBACK(on_load_model_clicked), data);
    gtk_box_append(GTK_BOX(box), load_model_button);
    GtkWidget *image_button = gtk_button_new_with_label("选择图像");
    g_signal_connect(image_button, "clicked", G_CALLBACK(on_image_button_clicked), data);
    gtk_box_append(GTK_BOX(box), image_button);
    std::cout << "[LOG] 所有控件已创建" << std::endl;
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), data);
    gtk_widget_set_visible(window, TRUE);
    std::cout << "[LOG] 窗口已设置为可见" << std::endl;
}

#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int argc = __argc;
    char **argv = __argv;
    std::ofstream app_log_file("app_log.log", std::ios::app);
    auto cout_buf = std::cout.rdbuf();
    std::cout.rdbuf(app_log_file.rdbuf());
#else
int main(int argc, char **argv) {
    // 日志重定向到app_log.log
    std::ofstream app_log_file("app_log.log", std::ios::app);
#endif
    GtkApplication *app = gtk_application_new("org.example.app", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
