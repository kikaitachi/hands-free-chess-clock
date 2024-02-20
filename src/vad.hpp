#ifndef VAD_H_
#define VAD_H_

#include "onnxruntime_cxx_api.h"

#include <vector>

class timestamp_t {
 public:
  int start;
  int end;

  // default + parameterized constructor
  timestamp_t(int start = -1, int end = -1)
      : start(start), end(end)
  {
  };

  // assignment operator modifies object, therefore non-const
  timestamp_t& operator=(const timestamp_t& a)
  {
      start = a.start;
      end = a.end;
      return *this;
  };

  // equality comparison. doesn't modify object. therefore const.
  bool operator==(const timestamp_t& a) const
  {
      return (start == a.start && end == a.end);
  };
};

class VoiceActivityDetector {
 public:
  int64_t window_size_samples;  // Assign when init, support 256 512 768 for 8k; 512 1024 1536 for 16k.

  VoiceActivityDetector(int sample_rate);
  void reset();
  void predict(std::vector<float>& input);

 public:  // TODO: make private
  Ort::Env env;
  Ort::SessionOptions session_options;
  std::shared_ptr<Ort::Session> session = nullptr;
  Ort::AllocatorWithDefaultOptions allocator;
  Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(
    OrtArenaAllocator, OrtMemTypeCPU);

  // model config
  int sample_rate;  //Assign when init support 16000 or 8000
  int sr_per_ms;   // Assign when init, support 8 or 16
  float threshold;
  int min_silence_samples; // sr_per_ms * #ms
  int min_silence_samples_at_max_speech; // sr_per_ms * #98
  int min_speech_samples; // sr_per_ms * #ms
  float max_speech_samples;
  int speech_pad_samples; // usually a

  //Output timestamp
  std::vector<timestamp_t> speeches;
  timestamp_t current_speech;

  // model states
  bool triggered = false;
  unsigned int temp_end = 0;
  unsigned int current_sample = 0;
  // MAX 4294967295 samples / 8sample per ms / 1000 / 60 = 8947 minutes
  int prev_end;
  int next_start = 0;

  // Onnx model
  // Inputs
  std::vector<Ort::Value> ort_inputs;

  std::vector<const char *> input_node_names = {"input", "sr", "h", "c"};
  std::vector<float> input;
  std::vector<int64_t> sr;
  unsigned int size_hc = 2 * 1 * 64; // It's FIXED.
  std::vector<float> _h;
  std::vector<float> _c;

  int64_t input_node_dims[2] = {};
  const int64_t sr_node_dims[1] = {1};
  const int64_t hc_node_dims[3] = {2, 1, 64};

  // Outputs
  std::vector<Ort::Value> ort_outputs;
  std::vector<const char *> output_node_names = {"output", "hn", "cn"};
};

#endif  // VAD_H_
