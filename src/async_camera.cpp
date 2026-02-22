// async_camera.cpp
#define PY_SSIZE_T_CLEAN
#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <cstring>

namespace py = pybind11;

class AsyncCamera {
public:
    AsyncCamera(int device_index,
                int width,
                int height,
                double fps = 60.0)
        : m_width(width),
          m_height(height),
          m_running(false),
          m_front_index(0)
    {
        m_cap.open(device_index);
        if (!m_cap.isOpened()) {
            throw std::runtime_error("Failed to open camera device");
        }

        m_cap.set(cv::CAP_PROP_FRAME_WIDTH,  width);
        m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
        if (fps > 0.0)
            m_cap.set(cv::CAP_PROP_FPS, fps);

        // Allocate two fixed buffers
        m_buffers[0] = cv::Mat(m_height, m_width, CV_8UC3);
        m_buffers[1] = cv::Mat(m_height, m_width, CV_8UC3);
    }

    ~AsyncCamera() {
        stop();
    }

    void start() {
        if (m_running) return;
        m_running = true;
        m_thread = std::thread(&AsyncCamera::capture_loop, this);
    }

    void stop() {
        if (!m_running) return;
        m_running = false;
        if (m_thread.joinable())
            m_thread.join();
        m_cap.release();
    }

    // Returns (ptr, height, width, frame_id)
    py::tuple get_latest_ptr() {
        int idx = m_front_index;
        uintptr_t ptr = reinterpret_cast<uintptr_t>(m_buffers[idx].data);
        return py::make_tuple(ptr, m_height, m_width, m_frame_id);
    }

private:
    void capture_loop() {
        cv::Mat frame;

        while (m_running) {

            if (!m_cap.read(frame))
                continue;

            // Force correct resolution once
            if (frame.cols != m_width || frame.rows != m_height) {
                cv::resize(frame, frame,
                           cv::Size(m_width, m_height));
            }

            int back = 1 - m_front_index.load();

            std::memcpy(
                m_buffers[back].data,
                frame.data,
                m_width * m_height * 3
            );

            m_front_index = back;
            m_frame_id++;
        }
    }

    int m_width;
    int m_height;

    cv::VideoCapture m_cap;
    std::thread m_thread;

    std::atomic<bool> m_running;
    std::atomic<int>  m_front_index;
    std::atomic<uint64_t> m_frame_id{0};

    cv::Mat m_buffers[2];
};

PYBIND11_MODULE(async_camera, m) {
    py::class_<AsyncCamera>(m, "AsyncCamera")
        .def(py::init<int,int,int,double>(),
             py::arg("device_index"),
             py::arg("width"),
             py::arg("height"),
             py::arg("fps") = 60.0)
        .def("start", &AsyncCamera::start)
        .def("stop",  &AsyncCamera::stop)
        .def("get_latest_ptr", &AsyncCamera::get_latest_ptr);
}