#include "G1MultiFlagsAdaptiveGCControl.hpp"

G1MultiFlagsAdaptiveGCControl ::G1MultiFlagsAdaptiveGCControl (uintx MGCT, uintx OCRTP, uintx MGLTP,
                                                    G1OldGenAllocationTracker const *old_gen_alloc_tracker,
                                                    G1Predictions const *predictor)
    : _mixed_gc_count_target(MGCT), 
      _mixed_gc_live_threshold(MGLTP),
      _old_gen_alloc_tracker(old_gen_alloc_tracker),
      _predictor(predictor),
      _marking_times_s(10, 0.05),
      _allocation_rate_s(10, 0.05),
      _garbage_rate_s(10, 0.05),
      _last_unrestrained_young_size(0) {}

uintx G1MultiFlagsAdaptiveGCControl ::getMixedGCCountTarget() {
    if (have_enough_data_for_prediction()) {
        double pred_marking_time = predict(&_marking_times_s);
        double pred_promotion_rate = predict(&_allocation_rate_s);
        double pred_garbage_rate = predict(&_garbage_rate_s);
        size_t pred_promotion_size = (size_t)(pred_marking_time * pred_promotion_rate);

        if (pred_promotion_rate > pred_garbage_rate) {
            if (_mixed_gc_count_target > 4) 
                _mixed_gc_count_target -= 1;
        } else {
            if (_mixed_gc_count_target < 12) 
                _mixed_gc_count_target += 1;
        }

        if (G1CollectedHeap::heap()->used() + pred_promotion_size > G1CollectedHeap::heap()->max_capacity()) {
            _mixed_gc_count_target = 1;
        }
    }
    return _mixed_gc_count_target;
}

uintx G1MultiFlagsAdaptiveGCControl ::getMixedGCLiveThresholdPercent() {
    if (have_enough_data_for_prediction()) {
        double pred_garbage_rate = predict(&_garbage_rate_s);

        if (pred_garbage_rate > 0.1) { // Adjust threshold based on garbage rate
            if (_mixed_gc_live_threshold < 100) 
                _mixed_gc_live_threshold += 5;
        } else {
            if (_mixed_gc_live_threshold > 0) 
                _mixed_gc_live_threshold -= 5;
        }

        if (G1CollectedHeap::heap()->used() + pred_garbage_rate > G1CollectedHeap::heap()->max_capacity()) {
            _mixed_gc_live_threshold = 100; // Force max threshold
        }
    }
    return _mixed_gc_live_threshold;
}

void G1MultiFlagsAdaptiveGCControl ::update_allocation_info(double allocation_time_s, size_t additional_buffer_size) {
    assert(allocation_time_s >= 0.0, "Allocation time must be positive");
    _last_allocation_time_s = allocation_time_s;
    _allocation_rate_s.add(last_mutator_period_old_allocation_rate());
    _last_unrestrained_young_size = additional_buffer_size;
}

void G1MultiFlagsAdaptiveGCControl ::update_marking_length(double marking_length_s) {
    assert(marking_length_s >= 0.0, "Marking length must be positive");
    _marking_times_s.add(marking_length_s);
}

void G1MultiFlagsAdaptiveGCControl ::update_garbage_info(double allocation_time_s, size_t garbage_bytes_s) {
    assert(allocation_time_s >= 0.0, "Allocation time must be positive");
    _last_allocation_time_s = allocation_time_s;
    _garbage_rate_s.add(garbage_bytes_s / _last_allocation_time_s);
}

bool G1MultiFlagsAdaptiveGCControl ::have_enough_data_for_prediction() const {
    return ((size_t)_marking_times_s.num() >= 3) &&
           ((size_t)_allocation_rate_s.num() >= 3);
}

double G1MultiFlagsAdaptiveGCControl ::predict(TruncatedSeq const* seq) const {
    return _predictor->predict_zero_bounded(seq);
}

double G1MultiFlagsAdaptiveGCControl ::last_mutator_period_old_allocation_rate() const {
    assert(_last_allocation_time_s > 0, "Invalid last allocation time");
    return _old_gen_alloc_tracker->last_period_old_gen_growth() / _last_allocation_time_s;
}
