// Buddy Allocator implementation without STL containers
// Fulfills the required interface in namespace sjtu

namespace sjtu {

class BuddyAllocator {
public:
  BuddyAllocator(int ram_size, int min_block_size)
      : ram_size_(ram_size), unit_(min_block_size) {
    // Compute number of minimal units (must be power of two)
    N_units_ = ram_size_ / unit_;
    // Compute max order such that unit_ << max_ord == ram_size_
    max_ord_ = 0;
    int tmp = N_units_;
    while ((tmp >>= 1)) ++max_ord_;

    // Complete binary tree with N_units_ leaves => total nodes < 2*N_units_
    tree_size_ = 2 * N_units_;
    longest_ = new int[tree_size_];
    node_size_ = new int[tree_size_];

    // Initialize node sizes (bytes) top-down
    node_size_[1] = ram_size_;
    for (int i = 2; i < tree_size_; ++i) {
      node_size_[i] = node_size_[i >> 1] >> 1; // half of parent
    }
    // Initially everything is free
    for (int i = 1; i < tree_size_; ++i) longest_[i] = node_size_[i];
  }

  int malloc(int size) {
    if (size <= 0) return -1;
    if (size % unit_ != 0) return -1;
    if (!is_power_of_two(size / unit_)) return -1;

    if (longest_[1] < size) return -1;

    int idx = 1;
    int cur_size = node_size_[idx];
    int addr = 0;

    while (cur_size != size) {
      int li = idx << 1, ri = li | 1;
      int half = cur_size >> 1;
      if (longest_[li] >= size) {
        idx = li;
        cur_size = half;
      } else {
        idx = ri;
        addr += half;
        cur_size = half;
      }
    }

    // Allocate this node
    longest_[idx] = 0;
    update_up(idx);
    return addr;
  }

  int malloc_at(int addr, int size) {
    if (size <= 0) return -1;
    if (addr < 0 || addr + size > ram_size_) return -1;
    if (size % unit_ != 0) return -1;
    if (addr % size != 0) return -1;
    if (!is_power_of_two(size / unit_)) return -1;

    int idx = locate(addr, size);
    if (idx <= 0) return -1;
    if (longest_[idx] != node_size_[idx]) return -1; // not fully free exactly here

    longest_[idx] = 0;
    update_up(idx);
    return addr;
  }

  void free_at(int addr, int size) {
    if (size <= 0) return;
    if (addr < 0 || addr + size > ram_size_) return;
    if (size % unit_ != 0) return;
    if (addr % size != 0) return;
    if (!is_power_of_two(size / unit_)) return;

    int idx = locate(addr, size);
    if (idx <= 0) return;

    longest_[idx] = node_size_[idx];
    update_up(idx);
  }

  ~BuddyAllocator() {
    delete[] longest_;
    delete[] node_size_;
  }

private:
  int ram_size_;
  int unit_;
  int N_units_;
  int max_ord_;
  int tree_size_;
  int *longest_;
  int *node_size_;

  static bool is_power_of_two(int x) { return x && ((x & (x - 1)) == 0); }

  void update_up(int idx) {
    while (idx > 1) {
      idx >>= 1; // move to parent
      int l = idx << 1, r = l | 1;
      int left_long = longest_[l];
      int right_long = longest_[r];
      int combined;
      // If both children are fully free, parent becomes fully free
      if (left_long == node_size_[l] && right_long == node_size_[r])
        combined = node_size_[idx];
      else
        combined = (left_long > right_long) ? left_long : right_long;
      longest_[idx] = combined;
    }
  }

  // Find the node index representing the block [addr, addr+size)
  int locate(int addr, int size) const {
    int idx = 1;
    int cur_size = node_size_[idx];
    int base = 0;
    while (cur_size != size) {
      int half = cur_size >> 1;
      if (addr < base + half) {
        idx = idx << 1;
        cur_size = half;
      } else {
        idx = (idx << 1) | 1;
        base += half;
        cur_size = half;
      }
    }
    return idx;
  }
};

} // namespace sjtu
