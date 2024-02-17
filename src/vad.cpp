#include "vad.hpp"

VoiceActivityDetector::VoiceActivityDetector(int sample_rate)
    : sample_rate(sample_rate) {
  session_options.SetIntraOpNumThreads(1);
  session_options.SetInterOpNumThreads(1);
  session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
  session = std::make_shared<Ort::Session>(
    env, "models/silero_vad.onnx", session_options);

  int windows_frame_size = 64;
  float Threshold = 0.5;
  int min_silence_duration_ms = 0;
  int speech_pad_ms = 64;
  int min_speech_duration_ms = 64;
  float max_speech_duration_s = std::numeric_limits<float>::infinity();

  threshold = Threshold;
  sr_per_ms = sample_rate / 1000;

  window_size_samples = windows_frame_size * sr_per_ms;

  min_speech_samples = sr_per_ms * min_speech_duration_ms;
  speech_pad_samples = sr_per_ms * speech_pad_ms;

  max_speech_samples = (
      sample_rate * max_speech_duration_s
      - window_size_samples
      - 2 * speech_pad_samples
      );

  min_silence_samples = sr_per_ms * min_silence_duration_ms;
  min_silence_samples_at_max_speech = sr_per_ms * 98;

  input.resize(window_size_samples);
  input_node_dims[0] = 1;
  input_node_dims[1] = window_size_samples;

  _h.resize(size_hc);
  _c.resize(size_hc);
  sr.resize(1);
  sr[0] = sample_rate;

  // Reset
  std::memset(_h.data(), 0.0f, _h.size() * sizeof(float));
  std::memset(_c.data(), 0.0f, _c.size() * sizeof(float));
  triggered = false;
  temp_end = 0;
  current_sample = 0;

  prev_end = next_start = 0;

  speeches.clear();
  current_speech = timestamp_t();
}

void VoiceActivityDetector::predict(std::vector<float>& data) {
  // Infer
  // Create ort tensors
  input.assign(data.begin(), data.end());
  Ort::Value input_ort = Ort::Value::CreateTensor<float>(
      memory_info, input.data(), input.size(), input_node_dims, 2);
  Ort::Value sr_ort = Ort::Value::CreateTensor<int64_t>(
      memory_info, sr.data(), sr.size(), sr_node_dims, 1);
  Ort::Value h_ort = Ort::Value::CreateTensor<float>(
      memory_info, _h.data(), _h.size(), hc_node_dims, 3);
  Ort::Value c_ort = Ort::Value::CreateTensor<float>(
      memory_info, _c.data(), _c.size(), hc_node_dims, 3);

  // Clear and add inputs
  ort_inputs.clear();
  ort_inputs.emplace_back(std::move(input_ort));
  ort_inputs.emplace_back(std::move(sr_ort));
  ort_inputs.emplace_back(std::move(h_ort));
  ort_inputs.emplace_back(std::move(c_ort));

  // Infer
  ort_outputs = session->Run(
      Ort::RunOptions{nullptr},
      input_node_names.data(), ort_inputs.data(), ort_inputs.size(),
      output_node_names.data(), output_node_names.size());

  // Output probability & update h,c recursively
  float speech_prob = ort_outputs[0].GetTensorMutableData<float>()[0];
  float *hn = ort_outputs[1].GetTensorMutableData<float>();
  std::memcpy(_h.data(), hn, size_hc * sizeof(float));
  float *cn = ort_outputs[2].GetTensorMutableData<float>();
  std::memcpy(_c.data(), cn, size_hc * sizeof(float));

  // Push forward sample index
  current_sample += window_size_samples;

  // Reset temp_end when > threshold
  if ((speech_prob >= threshold)) {
    if (temp_end != 0) {
        temp_end = 0;
        if (next_start < prev_end)
            next_start = current_sample - window_size_samples;
    }
    if (triggered == false) {
        triggered = true;
        current_speech.start = current_sample - window_size_samples;
    }
    return;
  }
  if (
      (triggered == true)
      && ((current_sample - current_speech.start) > max_speech_samples)
      ) {
      if (prev_end > 0) {
          current_speech.end = prev_end;
          speeches.push_back(current_speech);
          current_speech = timestamp_t();

          // previously reached silence(< neg_thres) and is still not speech(< thres)
          if (next_start < prev_end)
              triggered = false;
          else{
              current_speech.start = next_start;
          }
          prev_end = 0;
          next_start = 0;
          temp_end = 0;
      } else {
          current_speech.end = current_sample;
          speeches.push_back(current_speech);
          current_speech = timestamp_t();
          prev_end = 0;
          next_start = 0;
          temp_end = 0;
          triggered = false;
      }
      return;
  }
  if ((speech_prob >= (threshold - 0.15)) && (speech_prob < threshold)) {
    if (triggered) {
    } else {
    }
    return;
  }
  // 4) End
  if ((speech_prob < (threshold - 0.15)))
  {
    if (triggered == true) {
      if (temp_end == 0) {
          temp_end = current_sample;
      }
      if (current_sample - temp_end > min_silence_samples_at_max_speech)
        prev_end = temp_end;
      // a. silence < min_silence_samples, continue speaking
      if ((current_sample - temp_end) < min_silence_samples)
      {
      }
      // b. silence >= min_silence_samples, end speaking
      else {
          current_speech.end = temp_end;
          if (current_speech.end - current_speech.start > min_speech_samples) {
            speeches.push_back(current_speech);
            current_speech = timestamp_t();
            prev_end = 0;
            next_start = 0;
            temp_end = 0;
            triggered = false;
          }
      }
    }
    else {
        // may first windows see end state.
    }
    return;
  }
}
