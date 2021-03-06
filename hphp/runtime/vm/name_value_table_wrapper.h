/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#ifndef incl_HPHP_RUNTIME_VM_NAME_VALUE_TABLE_WRAPPER_H
#define incl_HPHP_RUNTIME_VM_NAME_VALUE_TABLE_WRAPPER_H

#include "hphp/runtime/vm/name_value_table.h"
#include "hphp/runtime/base/array/array_data.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Wrapper to provide a KindOfArray interface to a NameValueTable.
 * This is used to expose a NameValueTable to php code, particularly
 * for $GLOBALS, but also for some internal generated-bytecode
 * initialization routines (86pinit, 86sinit).
 *
 * Some differences compared to normal php arrays:
 *
 *   - Does not behave as if it has value semantics.  (I.e., no COW.)
 *
 *   - Iteration order is not specified.
 *
 *   - Non-string keys are not really supported.  (Integers are
 *     converted to strings.)
 *
 *   - size() is an O(N) operation.  (This is because of KindOfIndirect
 *     support in the underlying NameValueTable.)
 *
 *   - Append/prepend operations are not supported.
 *
 *   - Strong iterators "past the end" are not updated when new
 *     elements are added.  (Since iteration order is unspecified,
 *     this semantic would seem weird anyway.)
 *
 * This holds a pointer to a NameValueTable whose lifetime must be
 * guaranteed to outlast the lifetime of the NameValueTableWrapper.
 * (The wrapper is refcounted, as required by ArrayData, but the table
 * pointed to is not.)
 */
struct NameValueTableWrapper : public ArrayData {
  explicit NameValueTableWrapper(NameValueTable* tab)
    : ArrayData(ArrayKind::kNameValueTableWrapper)
    , m_tab(tab)
  { }

public: // ArrayData implementation

  // these using directives ensure the full set of overloaded functions
  // are visible in this class, to avoid triggering implicit conversions
  // from a CVarRef key to int64.
  using ArrayData::exists;
  using ArrayData::lval;
  using ArrayData::lvalNew;
  using ArrayData::set;
  using ArrayData::setRef;
  using ArrayData::add;
  using ArrayData::addLval;
  using ArrayData::remove;

  Variant& getRef(CStrRef k) {
    return tvAsVariant(nvGet(k.get()));
  }

  virtual ssize_t vsize() const;
  virtual Variant getKey(ssize_t pos) const;
  virtual Variant getValue(ssize_t pos) const;
  virtual CVarRef getValueRef(ssize_t pos) const;
  virtual bool noCopyOnWrite() const;

  virtual bool exists(int64_t k) const;
  virtual bool exists(const StringData* k) const;
  virtual bool idxExists(ssize_t idx) const;

  virtual TypedValue* nvGet(int64_t k) const;
  virtual TypedValue* nvGet(const StringData* k) const;

  virtual ArrayData* lval(int64_t k, Variant*& ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData* lval(StringData* k, Variant*& ret,
                          bool copy, bool checkExist = false);
  virtual ArrayData* lvalNew(Variant*& ret, bool copy);

  virtual ArrayData* set(int64_t k, CVarRef v, bool copy);
  virtual ArrayData* set(StringData* k, CVarRef v, bool copy);
  virtual ArrayData* setRef(int64_t k, CVarRef v, bool copy);
  virtual ArrayData* setRef(StringData* k, CVarRef v, bool copy);
  virtual ArrayData* remove(int64_t k, bool copy);
  virtual ArrayData* remove(const StringData* k, bool copy);

  virtual ArrayData* copy() const { return 0; }

  virtual ArrayData* append(CVarRef v, bool copy);
  virtual ArrayData* appendRef(CVarRef v, bool copy);
  virtual ArrayData* appendWithRef(CVarRef v, bool copy);

  virtual ArrayData* plus(const ArrayData* elems, bool copy);
  virtual ArrayData* merge(const ArrayData* elems, bool copy);

  virtual ArrayData* prepend(CVarRef v, bool copy);

  virtual ssize_t iter_begin() const;
  virtual ssize_t iter_end() const;
  virtual ssize_t iter_advance(ssize_t prev) const;
  virtual ssize_t iter_rewind(ssize_t prev) const;

  virtual Variant reset();
  virtual Variant prev();
  virtual Variant current() const;
  virtual Variant next();
  virtual Variant end();
  virtual CVarRef endRef();
  virtual Variant key() const;
  virtual Variant value(int32_t &pos) const;
  virtual Variant each();
  virtual bool validFullPos(const FullPos & fp) const;
  virtual bool advanceFullPos(FullPos&);
  virtual bool isVectorData() const;

  virtual ArrayData* escalateForSort();
  virtual void ksort(int sort_flags, bool ascending);
  virtual void sort(int sort_flags, bool ascending);
  virtual void asort(int sort_flags, bool ascending);
  virtual void uksort(CVarRef cmp_function);
  virtual void usort(CVarRef cmp_function);
  virtual void uasort(CVarRef cmp_function);

private:
  NameValueTable* const m_tab;
};

class GlobalNameValueTableWrapper : public NameValueTableWrapper {
 public:
  explicit GlobalNameValueTableWrapper(NameValueTable* tab);
};

//////////////////////////////////////////////////////////////////////

}

#endif
