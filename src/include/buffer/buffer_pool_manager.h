//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager.h
//
// Identification: src/include/buffer/buffer_pool_manager.h
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <list>
#include <memory>
#include <mutex>  // NOLINT
#include <unordered_map>

#include "buffer/lru_k_replacer.h"
#include "common/config.h"
#include "recovery/log_manager.h"
#include "storage/disk/disk_scheduler.h"
#include "storage/page/page.h"
#include "storage/page/page_guard.h"

namespace bustub {

// 缓冲池管理器类声明
class BufferPoolManager {
 public:
  // 构造函数，用于创建一个新的缓冲池管理器
  BufferPoolManager(size_t pool_size, DiskManager *disk_manager, size_t replacer_k = LRUK_REPLACER_K,
                    LogManager *log_manager = nullptr);

  // 析构函数，用于销毁缓冲池管理器
  ~BufferPoolManager();

  // 返回缓冲池的大小（帧数）
  auto GetPoolSize() -> size_t { return pool_size_; }

  // 返回所有缓冲池页面的指针
  auto GetPages() -> Page * { return pages_; }

  // 创建一个新页面并返回其指针
  auto NewPage(page_id_t *page_id) -> Page *;

  // 用于创建新页面并返回一个 BasicPageGuard 结构
  auto NewPageGuarded(page_id_t *page_id) -> BasicPageGuard;

  // 从缓冲池中获取页面或从磁盘中读取页面
  auto FetchPage(page_id_t page_id, AccessType access_type = AccessType::Unknown) -> Page *;

  // 用于获取页面的 PageGuard 包装器
  auto FetchPageBasic(page_id_t page_id) -> BasicPageGuard;
  auto FetchPageRead(page_id_t page_id) -> ReadPageGuard;
  auto FetchPageWrite(page_id_t page_id) -> WritePageGuard;

  // 从缓冲池中解除页面的引用
  auto UnpinPage(page_id_t page_id, bool is_dirty, AccessType access_type = AccessType::Unknown) -> bool;

  // 将页面刷新到磁盘
  auto FlushPage(page_id_t page_id) -> bool;

  // 刷新缓冲池中的所有页面到磁盘
  void FlushAllPages();

  // 从缓冲池中删除页面
  auto DeletePage(page_id_t page_id) -> bool;

 private:
  // 缓冲池的大小
  const size_t pool_size_;

  // 下一个要分配的页面ID
  std::atomic<page_id_t> next_page_id_;

  // 缓冲池页面数组
  Page *pages_;

  // 指向磁盘调度器的指针
  std::unique_ptr<DiskScheduler> disk_scheduler_;

  // 指向日志管理器的指针（仅用于测试，P1阶段可以忽略）
  // LogManager *log_manager_;

  // 页面表，用于跟踪缓冲池页面
  std::unordered_map<page_id_t, frame_id_t> page_table_;

  // 替换器，用于查找可以替换的未固定页面
  std::unique_ptr<LRUKReplacer> replacer_;

  // 空闲帧的链表，表示没有分配页面的帧
  std::list<frame_id_t> free_list_;

  // 保护共享数据结构的互斥锁
  std::mutex latch_;

  // 分配新页面的私有辅助函数
  auto AllocatePage() -> page_id_t;

  // 释放页面的私有辅助函数（当前没有实际实现）
  void DeallocatePage(__attribute__((unused)) page_id_t page_id) {
    // 当前无需实际操作，因为没有复杂的数据结构来跟踪已释放的页面
  }

  // TODO（学生）：您可以添加其他私有成员和辅助函数
};

}  // namespace bustub
