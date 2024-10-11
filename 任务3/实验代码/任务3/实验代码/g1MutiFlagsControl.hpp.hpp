#ifndef G1_ADAPTIVE_MIXEDGC_CONTROL_HPP
#define G1_ADAPTIVE_MIXEDGC_CONTROL_HPP

#include <cassert>
#include "gc/g1/g1CollectedHeap.inline.hpp"
#include "gc/g1/g1OldGenAllocationTracker.hpp"
#include "gc/g1/g1Predictions.hpp"

class G1MultiFlagsAdaptiveGCControl  {
public:
    G1MultiFlagsAdaptiveGCControl (uintx MGCT, uintx OCRTP, uintx MGLTP,
                              G1OldGenAllocationTracker const *old_gen_alloc_tracker,
                              G1Predictions const *predictor);
    
    uintx getMixedGCCountTarget();
    uintx getMixedGCLiveThresholdPercent();
    void update_allocation_info(double allocation_time_s, size_t additional_buffer_size);
    void update_marking_length(double marking_length_s);
    void update_garbage_info(double allocation_time_s, size_t garbage_bytes_s);

private:
    uintx _mixed_gc_count_target;
    uintx _old_cset_region_threshold_percent;
    uintx _mixed_gc_live_threshold; // New parameter for live threshold
    G1OldGenAllocationTracker const *_old_gen_alloc_tracker;
    G1Predictions const *_predictor;

    TruncatedSeq<double> _marking_times_s;
    TruncatedSeq<double> _allocation_rate_s;
    TruncatedSeq<double> _garbage_rate_s;

    size_t _last_unrestrained_young_size;

    bool have_enough_data_for_prediction() const;
    double predict(TruncatedSeq const* seq) const;
    double last_mutator_period_old_allocation_rate() const;
};

#endif // G1_ADAPTIVE_MIXEDGC_CONTROL_HPP
