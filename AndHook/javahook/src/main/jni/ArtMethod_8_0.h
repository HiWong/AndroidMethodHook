//
// Created by panda on 17/10/17.
//

#ifndef ANDHOOK_ARTMETHOD_8_0_H
#define ANDHOOK_ARTMETHOD_8_0_H
#include <stddef.h>
typedef unsigned char		uint8_t;
typedef unsigned short int	uint16_t;
typedef unsigned int		uint32_t;

class Object{
    // The number of vtable entries in java.lang.Object.
    static constexpr size_t kVTableLength = 11;
    static size_t hash_code_seed;

    // The Class representing the type of the object.
    uint32_t klass_;
    // Monitor and hash code information.
    uint32_t monitor_;

#ifdef USE_BROOKS_READ_BARRIER
    // Note names use a 'x' prefix and the x_rb_ptr_ is of type int
  // instead of Object to go with the alphabetical/by-type field order
  // on the Java side.
  uint32_t x_rb_ptr_;      // For the Brooks pointer.
  uint32_t x_xpadding_;    // For 8-byte alignment. TODO: get rid of this.
#endif

};
class Class:public Object{
public:
    static constexpr uint32_t kClassWalkSuper = 0xC0000000;

    // Shift primitive type by kPrimitiveTypeSizeShiftShift to get the component type size shift
    // Used for computing array size as follows:
    // array_bytes = header_size + (elements << (primitive_type >> kPrimitiveTypeSizeShiftShift))
    static constexpr uint32_t kPrimitiveTypeSizeShiftShift = 16;
    static constexpr uint32_t kPrimitiveTypeMask = (1u << kPrimitiveTypeSizeShiftShift) - 1;
    enum Status {
        kStatusRetired = -3,  // Retired, should not be used. Use the newly cloned one instead.
        kStatusErrorResolved = -2,
        kStatusErrorUnresolved = -1,
        kStatusNotReady = 0,
        kStatusIdx = 1,  // Loaded, DEX idx in super_class_type_idx_ and interfaces_type_idx_.
        kStatusLoaded = 2,  // DEX idx values resolved.
        kStatusResolving = 3,  // Just cloned from temporary class object.
        kStatusResolved = 4,  // Part of linking.
        kStatusVerifying = 5,  // In the process of being verified.
        kStatusRetryVerificationAtRuntime = 6,  // Compile time verification failed, retry at runtime.
        kStatusVerifyingAtRuntime = 7,  // Retrying verification at runtime.
        kStatusVerified = 8,  // Logically part of linking; done pre-init.
        kStatusInitializing = 9,  // Class init in progress.
        kStatusInitialized = 10,  // Ready to go.
        kStatusMax = 11,
    };
    enum {
        kDumpClassFullDetail = 1,
        kDumpClassClassLoader = (1 << 1),
        kDumpClassInitialized = (1 << 2),
    };
    class InitializeClassVisitor {

    private:
        const uint32_t class_size_;

    };
    // Defining class loader, or null for the "bootstrap" system loader.
    uint32_t class_loader_;

    // For array classes, the component class object for instanceof/checkcast
    // (for String[][][], this will be String[][]). null for non-array classes.
    uint32_t component_type_;

    // DexCache of resolved constant pool entries (will be null for classes generated by the
    // runtime such as arrays and primitive classes).
    uint32_t dex_cache_;

    // Extraneous class data that is not always needed. This field is allocated lazily and may
    // only be set with 'this' locked. This is synchronized on 'this'.
    // TODO(allight) We should probably synchronize it on something external or handle allocation in
    // some other (safe) way to prevent possible deadlocks.
    uint32_t ext_data_;

    // The interface table (iftable_) contains pairs of a interface class and an array of the
    // interface methods. There is one pair per interface supported by this class.  That means one
    // pair for each interface we support directly, indirectly via superclass, or indirectly via a
    // superinterface.  This will be null if neither we nor our superclass implement any interfaces.
    //
    // Why we need this: given "class Foo implements Face", declare "Face faceObj = new Foo()".
    // Invoke faceObj.blah(), where "blah" is part of the Face interface.  We can't easily use a
    // single vtable.
    //
    // For every interface a concrete class implements, we create an array of the concrete vtable_
    uint32_t iftable_;

    // Descriptor for the class such as "java.lang.Class" or "[C". Lazily initialized by ComputeName
    uint32_t name_;

    // The superclass, or null if this is java.lang.Object or a primitive type.
    //
    // Note that interfaces have java.lang.Object as their
    // superclass. This doesn't match the expectations in JNI
    // GetSuperClass or java.lang.Class.getSuperClass() which need to
    // check for interfaces and return null.
    uint32_t super_class_;

    // Virtual method table (vtable), for use by "invoke-virtual".  The vtable from the superclass is
    // copied in, and virtual methods from our class either replace those from the super or are
    // appended. For abstract classes, methods may be created in the vtable that aren't in
    // virtual_ methods_ for miranda methods.
    uint32_t vtable_;

    // instance fields
    //
    // These describe the layout of the contents of an Object.
    // Note that only the fields directly declared by this class are
    // listed in ifields; fields declared by a superclass are listed in
    // the superclass's Class.ifields.
    //
    // ArtFields are allocated as a length prefixed ArtField array, and not an array of pointers to
    // ArtFields.
    uint64_t ifields_;

    // Pointer to an ArtMethod length-prefixed array. All the methods where this class is the place
    // where they are logically defined. This includes all private, static, final and virtual methods
    // as well as inherited default methods and miranda methods.
    //
    // The slice methods_ [0, virtual_methods_offset_) are the direct (static, private, init) methods
    // declared by this class.
    //
    // The slice methods_ [virtual_methods_offset_, copied_methods_offset_) are the virtual methods
    // declared by this class.
    //
    // The slice methods_ [copied_methods_offset_, |methods_|) are the methods that are copied from
    // interfaces such as miranda or default methods. These are copied for resolution purposes as this
    // class is where they are (logically) declared as far as the virtual dispatch is concerned.
    //
    // Note that this field is used by the native debugger as the unique identifier for the type.
    uint64_t methods_;

    // Static fields length-prefixed array.
    uint64_t sfields_;

    // Access flags; low 16 bits are defined by VM spec.
    uint32_t access_flags_;

    // Class flags to help speed up visiting object references.
    uint32_t class_flags_;

    // Total size of the Class instance; used when allocating storage on gc heap.
    // See also object_size_.
    uint32_t class_size_;

    // Tid used to check for recursive <clinit> invocation.
    pid_t clinit_thread_id_;

    // ClassDef index in dex file, -1 if no class definition such as an array.
    // TODO: really 16bits
    int32_t dex_class_def_idx_;

    // Type index in dex file.
    // TODO: really 16bits
    int32_t dex_type_idx_;

    // Number of instance fields that are object refs.
    uint32_t num_reference_instance_fields_;

    // Number of static fields that are object refs,
    uint32_t num_reference_static_fields_;

    // Total object size; used when allocating storage on gc heap.
    // (For interfaces and abstract classes this will be zero.)
    // See also class_size_.
    uint32_t object_size_;

    // Aligned object size for allocation fast path. The value is max uint32_t if the object is
    // uninitialized or finalizable. Not currently used for variable sized objects.
    uint32_t object_size_alloc_fast_path_;

    // The lower 16 bits contains a Primitive::Type value. The upper 16
    // bits contains the size shift of the primitive type.
    uint32_t primitive_type_;

    // Bitmap of offsets of ifields.
    uint32_t reference_instance_offsets_;

    // State of class initialization.
    Status status_;

    // The offset of the first virtual method that is copied from an interface. This includes miranda,
    // default, and default-conflict methods. Having a hard limit of ((2 << 16) - 1) for methods
    // defined on a single class is well established in Java so we will use only uint16_t's here.
    uint16_t copied_methods_offset_;

    // The offset of the first declared virtual methods in the methods_ array.
    uint16_t virtual_methods_offset_;

};
class ArtMethod {
public:
// Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
// The class we are a part of.
    Class *declaring_class_;

// Access flags; low 16 bits are defined by spec.
// Getting and setting this flag needs to be atomic when concurrency is
// possible, e.g. after this method's class is linked. Such as when setting
// verifier flags and single-implementation flag.
    uint32_t access_flags_;

/* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */

// Offset to the CodeItem.
    uint32_t dex_code_item_offset_;

// Index into method_ids of the dex file associated with this method.
    uint32_t dex_method_index_;

/* End of dex file fields. */

// Entry within a dispatch table for this method. For static/direct methods the index is into
// the declaringClass.directMethods, for virtual methods the vtable and for interface methods the
// ifTable.
    uint16_t method_index_;

// The hotness we measure for this method. Managed by the interpreter. Not atomic, as we allow
// missing increments: if the method is hot, we will see it eventually.
    uint16_t hotness_count_;

// Fake padding field gets inserted here.

// Must be the last fields in the method.
    struct PtrSizedFields {
        // Short cuts to declaring_class_->dex_cache_ member for fast compiled code access.
        ArtMethod **dex_cache_resolved_methods_;

        // Pointer to JNI function registered to this method, or a function to resolve the JNI function,
        // or the profiling data for non-native methods, or an ImtConflictTable, or the
        // single-implementation of an abstract/interface method.
        void *data_;

        // Method dispatch from quick compiled code invokes this pointer which may cause bridging into
        // the interpreter.
        void *entry_point_from_quick_compiled_code_;
    } ptr_sized_fields_;
};
static constexpr uint32_t kAccPublic =       0x0001;  // class, field, method, ic
static constexpr uint32_t kAccPrivate =      0x0002;  // field, method, ic
static constexpr uint32_t kAccProtected =    0x0004;  // field, method, ic
static constexpr uint32_t kAccStatic =       0x0008;  // field, method, ic
static constexpr uint32_t kAccConstructor =           0x00010000;  // method (dex only) <(cl)init>
static constexpr uint32_t kAccNative =       0x0100;  // method
static constexpr uint32_t kAccFinal =        0x0010;
static bool IsDirect(uint32_t access_flags) {
    constexpr uint32_t direct = kAccStatic | kAccPrivate | kAccConstructor;
    return (access_flags & direct) != 0;
}
inline bool IsSubClass(Class* thiz,Class* klass) {
    Class* current = thiz;
    do {
        if (current == klass) {
            return true;
        }
        current = (Class* )current->super_class_;
    } while (current != nullptr);
    return false;
}
bool IsFinal(uint32_t access_flags){
        return (access_flags & kAccFinal) != 0;
}
#endif //ANDHOOK_ARTMETHOD_8_0_H