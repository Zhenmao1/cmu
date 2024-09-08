//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer.h
//
// 文件标识：src/include/buffer/lru_k_replacer.h
//
// 版权所有 (c) 2015-2022，卡内基梅隆大学数据库组
//
//===----------------------------------------------------------------------===//

#pragma once

#include <limits>
#include <list>
#include <mutex>  // NOLINT
#include <unordered_map>
#include <vector>

#include "common/config.h"
#include "common/macros.h"

namespace bustub {

// 访问类型枚举，用于记录访问类型
enum class AccessType { Unknown = 0, Lookup, Scan, Index };

// LRUKNode 类，表示LRU-K替换策略中的节点
class LRUKNode {
public:
  // 存储该页面最近K次访问的时间戳历史记录，最早的时间戳在列表前面
  std::list<size_t> history_;
  frame_id_t fid_; // 页面的帧ID
  size_t k_;//这里应该解释为k值，不是K次访问记录的K，而是K次访问的最小K
  bool is_evictable_{false}; // 标识该页面是否可以被替换
public:
  LRUKNode() {fid_=-1;k_=0;}
  LRUKNode(size_t time,frame_id_t id,bool evictable){
    history_.push_front(time);
    fid_=id;
    is_evictable_=evictable;
  }
  size_t GetBackKTimeStamp(){
    if(history_.size()==0) {
      return std::numeric_limits<size_t>::max();
    }
    return *history_.begin();
  }

};

/**
 * LRUKReplacer 类实现了LRU-k替换策略。
 *
 * LRU-k算法会替换掉具有最大反向k距离的页面，
 * 反向k距离是当前时间戳与第k次之前访问的时间戳之差。
 *
 * 具有少于k个历史引用的页面的反向k距离被视为+inf。当多个页面的反向k距离都是+inf时，
 * 将使用经典的LRU算法选择替换的页面。
 */
class LRUKReplacer {
 public:
  /**
   * 构造函数，用于初始化LRUKReplacer。
   *
   * @param num_frames LRUKReplacer需要存储的最大帧数
   * @param k K值
   */
  explicit LRUKReplacer(size_t num_frames, size_t k);

  DISALLOW_COPY_AND_MOVE(LRUKReplacer);

  /**
   * 析构函数，用于销毁LRUKReplacer。
   */
  ~LRUKReplacer() = default;

  /**
   * 找到具有最大反向k距离的页面并替换掉该页面。只有标记为“可替换”的页面才是替换候选页面。
   * 具有少于k个历史引用的页面的反向k距离被视为+inf。如果多个页面的反向k距离都是+inf，
   * 则基于LRU选择具有最早时间戳的页面。
   * 成功替换页面后，应该减少替换器的大小并删除页面的访问历史记录。
   * @param[out] frame_id 被替换的帧的ID
   * @return 如果成功替换页面，则返回true，如果没有可替换的页面，则返回false。
   */
  auto Evict(frame_id_t *frame_id) -> bool;

  /**
   * 记录给定帧ID在当前时间戳被访问的事件。
   * 如果帧ID无效（即大于replacer_size_），则抛出异常。
   *
   * @param frame_id 收到新访问的帧的ID
   * @param access_type 访问的类型，仅在领导板测试中需要此参数。
   */
  void RecordAccess(frame_id_t frame_id, AccessType access_type = AccessType::Unknown);

  /**
   * 切换帧是否可替换的状态。此函数还控制替换器的大小。
   *
   * 如果先前可替换的帧将其设置为不可替换，则大小应减少。
   * 如果先前不可替换的帧将其设置为可替换，则大小应增加。
   *
   * 如果帧ID无效，请抛出异常或中止进程。
   *
   * 对于其他情况，此函数应在不修改任何内容的情况下终止。
   *
   * @param frame_id 要修改“可替换”状态的帧的ID
   * @param set_evictable 是否允许替换给定的帧
   */
  void SetEvictable(frame_id_t frame_id, bool set_evictable);

  /**
   * 从替换器中移除一个可替换的帧，以及其访问历史记录。
   * 如果移除成功，此函数还应减小替换器的大小。
   *
   * 请注意，这与替换帧是不同的，替换总是删除具有最大反向k距离的帧。
   * 此函数会删除指定的帧ID，
   * 无论其反向k距离如何。
   * 如果Remove被调用于不可替换的帧上，请抛出异常或中止进程。
   * 如果未找到指定的帧，则直接从此函数返回。
   * @param frame_id 要移除的帧的ID
   */
  void Remove(frame_id_t frame_id);

  /**
   * 返回替换器的大小，该大小跟踪可替换的帧数。
   * @return size_t
   */
  auto Size() -> size_t;

 private:
  // TODO(student): 根据需要实现这些成员变量，如果开始使用它们，请删除maybe_unused。
  // 帧id->node的存储映射，也即是页表
  std::unordered_map<frame_id_t, LRUKNode> node_store_;
  // 如果lru node少于k 中不空，优先踢出
  std::list<LRUKNode> less_than_k;
  // 
  std::list<LRUKNode> more_than_k;
  // 当前的时间戳，需要手动，原因是系统的时钟在多进程可能有错
  size_t current_timestamp_{0};
  // 当前存放的可驱逐页面数量
  size_t curr_size_{0};
  // 整个主存大小（用于判断页是否非法越界）
  size_t replacer_size_={0};
  size_t k_;

  std::mutex latch_;
};

}  // namespace bustub
