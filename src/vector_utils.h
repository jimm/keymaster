#ifndef VECTOR_UTILS_H
#define VECTOR_UTILS_H

// Insert new_item after existing_item in vector v.
template <typename T>
void insert_after(vector<T> &v, T existing_item, T new_item) {
  for (typename vector<T>::iterator iter = v.begin(); iter != v.end(); ++iter) {
    if (*iter == existing_item) {
      v.insert(++iter, new_item);
      return;
    }
  }
}

// If item is inside vector v, remove it and close the gap.
// Assumes item only appears once in vector.
template <typename T>
void erase(vector<T> &v, T item) {
  for (typename vector<T>::iterator iter = v.begin(); iter != v.end(); ++iter) {
    if (*iter == item) {
      v.erase(iter);
      return;
    }
  }
}

#endif /* VECTOR_UTILS_H */
