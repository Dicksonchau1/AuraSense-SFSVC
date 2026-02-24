#pragma once

class DetectionScheduler {
public:
    DetectionScheduler();

    bool   should_detect(float crack_score);
    void   reset();
    void   set_base_interval_ms(double interval_ms);

private:
    double base_interval_ms_;
    double last_detect_ms_;
};
