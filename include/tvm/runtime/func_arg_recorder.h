//
// Created by hmc on 2021/2/26.
//

#ifndef TVM_FUNC_ARG_RECORDER_H
#define TVM_FUNC_ARG_RECORDER_H

#include <tvm/runtime/module.h>
#include <dmlc/json.h>
#include <queue>

namespace tvm {
namespace runtime {


class FunctionArgRecorder : public ModuleNode {
public:
  const char* type_key() const final { return "FunctionArgRecorder";}

  PackedFunc GetFunction(const std::string& name,
                        const ObjectPtr<Object>& sptr_to_self) override;

  void SaveToFile(const std::string& file_name, const std::string& format) override;

  void NewHostRecord(std::string name, std::vector<void*> args);
  void AllocArg(void* arg, size_t size);

  void NewDeviceRecord(std::string name, std::vector<void*> args, size_t launch_params[6]);

  void Print();

 std::string ToJson();
private:
  class DeviceRecord {
   public:
    std::string device_func_name;
    std::vector<void*> device_args;
    size_t launch_params[6];
    std::vector<int> device_arg_idx;
    void Save(dmlc::JSONWriter* writer) const {
      writer->BeginObject();
      writer->WriteObjectKeyValue("name", this->device_func_name);
      std::vector<size_t> params;
      for (int i = 0; i < 6; i++) params.push_back(launch_params[i]);
      writer->WriteObjectKeyValue("launch_params", params);
      writer->WriteObjectKeyValue("args", device_arg_idx);
      writer->EndObject();
    }
  };
  class HostRecord {
   public:
    std::string host_func_name;
    std::unordered_map<void*, size_t> host_args; // arg_ptr -> arg_idx
    std::unordered_map<void*, size_t> alloc_args; // arg_ptr -> arg_size
    std::vector<DeviceRecord> device_records;
    void Save(dmlc::JSONWriter* writer) const {
      writer->BeginObject();
      writer->WriteObjectKeyValue("name", host_func_name);
      writer->WriteObjectKeyValue("kernels", device_records);
      writer->EndObject();
    }
  };
  std::vector<HostRecord> records;

  std::vector<size_t> getTempArgs() {
    std::vector<std::priority_queue<size_t>> tmp;
    std::vector<size_t> temp_args;
    for (auto &host : records) {
      std::priority_queue<size_t> host_queue;
      for (auto alloc_arg : host.alloc_args) {
        host_queue.push(alloc_arg.second);
      }
     tmp.push_back(host_queue);
    }

    while (true) {
      size_t max = 0;
      for (auto& queue : tmp) {
        if (queue.empty()) continue;
         size_t top = queue.top();
         queue.pop();
        if (max < top) max = top;
      }
      if (max == 0) break;
      temp_args.push_back(max);
    }
    return temp_args;
  }

  void prepare_device_arg_idx() {
    for (auto &host : records) {
      // sort the host allocated args by size.
      std::vector<std::pair<void*, size_t>> sorted_array;
      for (auto p : host.alloc_args)
        sorted_array.push_back(p);
      std::sort(sorted_array.begin(), sorted_array.end(), [](const std::pair<void*, size_t> &l, const std::pair<void*, size_t> &r) {
        return l.second > r.second;
      });
      std::unordered_map<void*, int> alloc_arg_idx;
      for (size_t i = 0; i < sorted_array.size(); i++)
        alloc_arg_idx[sorted_array[i].first] = i;

      for (auto &dev : host.device_records) {
        for (auto &arg : dev.device_args) {
          if (host.host_args.find(arg) != host.host_args.end()) {
            dev.device_arg_idx.push_back((int)host.host_args[arg]);
          } else if (alloc_arg_idx.find(arg) != alloc_arg_idx.end()) {
            dev.device_arg_idx.push_back(-(alloc_arg_idx[arg] + 1));
          } else {
            LOG(FATAL) << "DEV ARG ERROR";
          }
        }
      }
    }
  }
}; // class FunctionArgRecorder

extern FunctionArgRecorder global_recorder;

} // namespace runtime


} // namespace tvm
#endif  // TVM_FUNC_ARG_RECORDER_H
