/** \file

    \mainpage

    This is a simple C++ sequential OpenCL SYCL C++ header file to
    experiment with the OpenCL CL provisional specification.

    For more information about OpenCL SYCL:
    http://www.khronos.org/opencl/sycl/

    The aim of this file is mainly to define the interface of SYCL so that
    the specification documentation can be derived from it through tools
    like Doxygen or Sphinx. This explains why there are many functions and
    classes that are here only to do some forwarding in some inelegant way.
    This file is documentation driven and not implementation-style driven.

    For more information on this project and to access to the source of
    this file, look at https://github.com/amd/triSYCL

    The Doxygen version of the API in
    http://amd.github.io/triSYCL/Doxygen/SYCL/html and
    http://amd.github.io/triSYCL/Doxygen/SYCL/SYCL-API-refman.pdf

    The Doxygen version of the implementation itself is in
    http://amd.github.io/triSYCL/Doxygen/triSYCL/html and
    http://amd.github.io/triSYCL/Doxygen/triSYCL/triSYCL-implementation-refman.pdf


    Ronan.Keryell at AMD point com

    Copyright 2014 Advanced Micro Devices, Inc.

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/


/* To remove some implementation details from the SYCL API documentation,
   rely on the preprocessor when this preprocessor symbol is defined */
#ifdef TRISYCL_HIDE_IMPLEMENTATION
// Remove the content of TRISYCL_IMPL...
#define TRISYCL_IMPL(...)
#else
// ... or keep the content of TRISYCL_IMPL
#define TRISYCL_IMPL(...) __VA_ARGS__
#endif

#include <cstddef>
#include <initializer_list>


/** Define TRISYCL_OPENCL to add OpenCL

    triSYCL can indeed work without OpenCL if only host support is needed.

    Right now it is set by Doxygen to generate the documentation.

    \todo Use a macro to check instead if the OpenCL header has been
    included before.

    But what is the right one? __OPENCL_CL_H? __OPENCL_C_VERSION__? CL_HPP_?
    Mostly CL_HPP_ to be able to use param_traits<> from cl.hpp...
*/
#ifdef TRISYCL_OPENCL
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#endif


/** The vector type to be used as SYCL vector

    \todo this should be more local, such as SYCL_VECTOR_CLASS or
    _SYCL_VECTOR_CLASS

    \todo use a typedef or a using instead of a macro?

    \todo implement __NO_STD_VECTOR

    \todo Table 3.1 in provisional specification is wrong: VECTOR_CLASS
    not at the right place
 */
#define VECTOR_CLASS std::vector


/** The string type to be used as SYCL string

    \todo this should be more local, such as SYCL_STRING_CLASS or
    _SYCL_STRING_CLASS

    \todo use a typedef or a using instead of a macro?

    \todo implement __NO_STD_STRING

    \todo Table 3.2 in provisional specification is wrong: STRING_CLASS
    not at the right place
 */
#define STRING_CLASS std::string


// SYCL dwells in the cl::sycl namespace
namespace cl {
namespace sycl {

/** \addtogroup data Data access and storage in SYCL

    @{
*/

/** Describe the type of access by kernels.

    \todo This values should be normalized to allow separate compilation
    with different implementations?
*/
namespace access {
  /* By using "enum mode" here instead of "enum struct mode", we have for
     example "write" appearing both as cl::sycl::access::mode::write and
     cl::sycl::access::write, instead of only the last one. This seems
     more conform to the specification. */

  /// This describes the type of the access mode to be used via accessor
  enum mode {
    read = 42, //< Why not? Insist on the fact that read_write != read + write
    write,
    atomic,
    read_write,
    discard_read_write
  };


  /** The target enumeration describes the type of object to be accessed
     via the accessor
   */
  enum target {
    global_buffer = 2014, //< Just pick a random number...
    constant_buffer,
    local,
    image,
    host_buffer,
    host_image,
    image_array,
    cl_buffer,
    cl_image
  };


  /** Precise the address space a barrier needs to act on
   */
  enum class address_space : char {
    local,
    global,
    global_and_local
  };

}

/// \todo implement image
template <std::size_t dimensions> struct image;

/// @} End the data Doxygen group

/** \addtogroup address_spaces Dealing with OpenCL address spaces
    @{
*/

/** Enumerate the different OpenCL 2 address spaces */
enum address_space {
  constant_address_space,
  generic_address_space,
  global_address_space,
  local_address_space,
  private_address_space,
};

/// @} End the address_spaces Doxygen group

}
}


// Include the implementation details
#include "implementation/sycl-implementation.hpp"


/// SYCL dwells in the cl::sycl namespace
namespace cl {
namespace sycl {

using namespace trisycl;

/** \addtogroup parallelism Expressing parallelism through kernels
    @{
*/

/** A SYCL range defines a multi-dimensional index range that can
    be used to launch parallel computation.

    \todo use std::size_t dims instead of int dims in the specification?

    \todo add to the specification this default parameter value?

    \todo add to the specification some way to specify an offset?
*/
template <std::size_t dims = 1>
struct range : public SmallArray123<std::size_t, range<dims>, dims> {
  // Inherit of all the constructors
  using SmallArray123<std::size_t, range<dims>, dims>::SmallArray123;
};


/** Implement a make_range to construct a range<> of the right dimension
    with implicit conversion from an initializer list for example.

    Cannot use a template on the number of dimensions because the implicit
    conversion would not be tried. */
auto make_range(range<1> r) { return r; }
auto make_range(range<2> r) { return r; }
auto make_range(range<3> r) { return r; }


/** Construct a range<> from a function call with arguments, like
    make_range(1, 2, 3) */
template<typename... BasicType>
auto make_range(BasicType... Args) {
  return range<sizeof...(Args)>(Args...);
}


/** Define a multi-dimensional index, used for example to locate a work item

    \todo The definition of id and item seem completely broken in the
    current specification. The whole 3.4.1 is to be updated.

    \todo It would be nice to have [] working everywhere, provide both
    get_...() and get_...(int dim) equivalent to get_...()[int dim]
    Well it is already the case for item. So not needed for id?
    Indeed [] is mentioned in text of page 59 but not in class description.
*/
template <std::size_t dims = 1>
struct id : public SmallArray123<std::ptrdiff_t, id<dims>, dims> {
  // Inherit of all the constructors
  using SmallArray123<std::ptrdiff_t, id<dims>, dims>::SmallArray123;
};


/** Implement a make_id to construct an id<> of the right dimension with
    implicit conversion from an initializer list for example.

    Cannot use a template on the number of dimensions because the implicit
    conversion would not be tried. */
auto make_id(id<1> i) { return i; }
auto make_id(id<2> i) { return i; }
auto make_id(id<3> i) { return i; }


/** Construct an id<> from a function call with arguments, like
    make_id(1, 2, 3) */
template<typename... BasicType>
auto make_id(BasicType... Args) {
  return id<sizeof...(Args)>(Args...);
}


/** A ND-range, made by a global and local range, to specify work-group
    and work-item organization.

    The local offset is used to translate the iteration space origin if
    needed.

    \todo add copy constructors in the specification
*/
template <std::size_t dims = 1>
struct nd_range {
  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static const auto dimensionality = dims;

private:

  range<dimensionality> GlobalRange;
  range<dimensionality> LocalRange;
  id<dimensionality> Offset;

public:

  /** Construct a ND-range with all the details available in OpenCL

      By default use a zero offset, that is iterations start at 0
   */
  nd_range(range<dims> global_size,
           range<dims> local_size,
           id<dims> offset = id<dims>()) :
    GlobalRange(global_size), LocalRange(local_size), Offset(offset) { }


  /// Get the global iteration space range
  range<dims> get_global_range() const { return GlobalRange; }


  /// Get the local part of the iteration space range
  range<dims> get_local_range() const { return LocalRange; }


  /// Get the range of work-groups needed to run this ND-range
  auto get_group_range() const {
    // \todo Assume that GlobalRange is a multiple of LocalRange, element-wise
    return GlobalRange/LocalRange;
  }


  /// \todo get_offset() is lacking in the specification
  id<dims> get_offset() const { return Offset; }


  /// Display the value for debugging and validation purpose
  void display() const {
    GlobalRange.display();
    LocalRange.display();
    Offset.display();
  }

};


/** A SYCL item stores information on a work-item with some more context
    such as the definition range and offset.
*/
template <std::size_t dims = 1>
struct item {
  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static const auto dimensionality = dims;

private:

  range<dims> Range;
  id<dims> GlobalIndex;
  id<dims> Offset;


public:

  /** Create an item from a local size and an optional offset

      \todo what is the meaning of this constructor for a programmer?
  */
  item(range<dims> global_size,
       id<dims> global_index,
       id<dims> offset = id<dims>()) :
    Range { global_size }, GlobalIndex { global_index }, Offset { offset } {}


  /** To be able to copy and assign item, use default constructors also

      \todo Make most of them protected, reserved to implementation
  */
  item() = default;

  /// Get the whole global id coordinate
  id<dims> get_global_id() const { return GlobalIndex; }


  /// Return the global coordinate in the given dimension
  size_t get(int dimension) const { return GlobalIndex[dimension]; }


  /// Return an l-value of the global coordinate in the given dimension
  auto &operator[](int dimension) { return GlobalIndex[dimension]; }


  /// Get the global range where this item dwells in
  range<dims> get_global_range() const { return Range; }


  /// Get the offset associated with the item context
  id<dims> get_offset() const { return Offset; }


  /** For the implementation, need to set the global index

      \todo Move to private and add friends
  */
  void set_global(id<dims> Index) { GlobalIndex = Index; }


  /// Display the value for debugging and validation purpose
  void display() const {
    Range.display();
    GlobalIndex.display();
    Offset.display();
  }

};


/** A SYCL nd_item stores information on a work-item within a work-group,
    with some more context such as the definition ranges.
*/
template <std::size_t dims = 1>
struct nd_item {
  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static const auto dimensionality = dims;

private:

  id<dims> GlobalIndex;
  id<dims> LocalIndex;
  nd_range<dims> NDRange;

public:

  /** Create an nd_item from a local size and local size

      \todo what is the meaning of this constructor for a programmer?
  */
  nd_item(range<dims> global_size, range<dims> local_size) :
    NDRange { global_size, local_size } {}


  /** \todo a constructor from a nd_range too in the specification if the
      previous one has a meaning?
   */
  nd_item(nd_range<dims> ndr) : NDRange { ndr } {}


  /** Create a full nd_item

      \todo this is for validation purpose. Hide this to the programmer
      somehow
  */
  nd_item(id<dims> global_index,
          id<dims> local_index,
          nd_range<dims> ndr) :
    GlobalIndex { global_index }, LocalIndex { local_index }, NDRange { ndr } {}


  /// Get the whole global id coordinate
  id<dims> get_global_id() const { return GlobalIndex; }


  /// Get the whole local id coordinate (which is respective to the
  /// work-group)
  id<dims> get_local_id() const { return LocalIndex; }


  /// Get the whole group id coordinate
  id<dims> get_group_id() const { return get_global_id()/get_local_range(); }


  /// Return the global coordinate in the given dimension
  auto get_global_id(int dimension) const { return get_global_id()[dimension]; }


  /// Return the local coordinate (that is in the work-group) in the given
  /// dimension
  auto get_local_id(int dimension) const { return get_local_id()[dimension]; }


  /// Get the whole group id coordinate in the given dimension
  id<dims> get_group_id(int dimension) const {
    return get_group_id()[dimension];
  }


  /// Get the global range where this nd_item dwells in
  range<dims> get_global_range() const { return NDRange.get_global_range(); }


  /// Get the local range (the dimension of the work-group) for this nd_item
  range<dims> get_local_range() const { return NDRange.get_local_range(); }


  /// Get the offset of the NDRange
  id<dims> get_offset() const { return NDRange.get_offset(); }


  /// Get the NDRange for this nd_item
  nd_range<dims> get_nd_range() const { return NDRange; }


  /** Executes a barrier with memory ordering on the local address space,
      global address space or both based on the value of flag. The current
      work- item will wait at the barrier until all work-items in the
      current work-group have reached the barrier.  In addition the
      barrier performs a fence operation ensuring that all memory accesses
      in the specified address space issued before the barrier complete
      before those issued after the barrier.

      \todo To be implemented
  */
  void barrier(access::address_space flag) const {}


  // For the implementation, need to set the local index
  void set_local(id<dims> Index) { LocalIndex = Index; }


  // For the implementation, need to set the global index
  void set_global(id<dims> Index) { GlobalIndex = Index; }

};


/** A group index used in a parallel_for_workitem to specify a work_group
 */
template <std::size_t dims = 1>
struct group {
  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static const auto dimensionality = dims;

private:

  /// Keep a reference on the nd_range to serve potential query on it
  nd_range<dims> NDR;
  /// The coordinate of the group item
  id<dims> Id;

public:

  /** Create a group from an nd_range<> with a 0 id<>

      \todo This should be private
  */
  group(const nd_range<dims> &ndr) : NDR(ndr) {}


  /** Create a group from an nd_range<> with a 0 id<>

      \todo This should be private
  */
  group(const nd_range<dims> &ndr, const id<dims> &i) :
    NDR(ndr), Id(i) {}


  /// Get the group identifier for this work_group
  id<dims> get_group_id() const { return Id; }


  /// Get the local range for this work_group
  range<dims> get_local_range() const { return NDR.get_local_range(); }


  /// Get the local range for this work_group
  range<dims> get_global_range() const { return NDR.get_global_range(); }


  /// Get the offset of the NDRange
  id<dims> get_offset() const { return NDR.get_offset(); }


  /// \todo Also provide this access to the current nd_range
  nd_range<dims> get_nd_range() const { return NDR; }


  /** Return the group coordinate in the given dimension

      \todo In this implementation it is not const because the group<> is
      written in the parallel_for iterators. To fix according to the
      specification
   */
  auto &operator[](int dimension) {
    return Id[dimension];
  }


  /// Return the group coordinate in the given dimension
  std::size_t get(int dimension) const {
    return Id[dimension];
  }

};

/// @} End the parallelism Doxygen group

/* Forward definitions (outside the Doxygen addtogroup to avoid multiple
   definitions) */
struct queue;

  template <typename T, std::size_t dimensions> struct buffer;


/** \addtogroup error_handling Error handling
    @{
*/

/**
   Encapsulate a SYCL error information
*/
struct exception {
#ifdef TRISYCL_OPENCL
  /** Get the OpenCL error code

      \returns 0 if not an OpenCL error

      \todo to be implemented
  */
  cl_int get_cl_code() { assert(0); }


  /** Get the SYCL-specific error code

      \returns 0 if not a SYCL-specific error

      \todo to be implemented

      \todo use something else instead of cl_int to be usable without
      OpenCL
  */
  cl_int get_sycl_code() { assert(0); }
#endif

  /** Get the queue that caused the error

      \return nullptr if not a queue error

      \todo Update specification to replace 0 by nullptr
  */
  queue *get_queue() { assert(0); }


  /** Get the buffer that caused the error

      \returns nullptr if not a buffer error

      \todo Update specification to replace 0 by nullptr and add the
      templated buffer

      \todo to be implemented
  */
  template <typename T, int dimensions> buffer<T, dimensions> *get_buffer() {
    assert(0); }


  /** Get the image that caused the error

      \returns nullptr if not a image error

      \todo Update specification to replace 0 by nullptr and add the
      templated buffer

      \todo to be implemented
  */
  template <std::size_t dimensions> image<dimensions> *get_image() { assert(0); }
};


namespace trisycl {
  // Create a default error handler to be used when nothing is specified
  struct default_error_handler;
}


/** User supplied error handler to call a user-provided function when an
    error happens from a SYCL object that was constructed with this error
    handler
*/
struct error_handler {
  /** The method to define to be called in the case of an error

      \todo Add "virtual void" to the specification
  */
  virtual void report_error(exception &error) = 0;

  /** Add a default_handler to be used by default

      \todo add this concept to the specification?
  */
  static trisycl::default_error_handler default_handler;
};


namespace trisycl {

  struct default_error_handler : error_handler {

    void report_error(exception &error) override {
    }
  };
}

  // \todo finish initialization
  //error_handler::default_handler = nullptr;

/// @} End the error_handling Doxygen group


/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** SYCL device

    \todo The implementation is quite minimal for now. :-)
*/
struct device {
  device() {}
};


/** The SYCL heuristics to select a device

    The device with the highest score is selected
*/
struct device_selector {
  // The user-provided operator computing the score
  virtual int operator() (device dev) = 0;
};


/** Select the best GPU, if any

    \todo to be implemented

    \todo to be named device_selector::gpu instead in the specification?

    \todo it is named opencl_gpu_selector
*/
struct gpu_selector : device_selector {
  // The user-provided operator computing the score
  int operator() (device dev) override { return 1; }
};


/** SYCL context

    The implementation is quite minimal for now. :-)
*/
struct context {
  context() {}

  // \todo fix this implementation
  context(gpu_selector s) {}

  context(device_selector &s) {}
};


/** SYCL queue, similar to the OpenCL queue concept.

    \todo The implementation is quite minimal for now. :-)
*/
struct queue {
  queue() {}

  queue(const context c) {}

  queue(const device_selector &s) {}
};


/** Abstract the OpenCL platform

    \todo triSYCL Implementation
*/
struct platform {

  /** Construct a default platform and provide an optional error_handler
      to deals with errors

      \todo Add copy/move constructor to the implementation

      \todo Add const to the specification
  */
  platform(const error_handler &handler = error_handler::default_handler) {}

#ifdef TRISYCL_OPENCL
  /** Create a SYCL platform from an existing OpenCL one and provide an
      optional error_handler to deals with errors

      \todo improve specification to accept also a cl.hpp object
  */
  platform(cl_platform_id platform id,
           const error_handler &handler = error_handler::default_handler) {}

  /** Create a SYCL platform from an existing OpenCL one and provide an
      integer place-holder to return the OpenCL error code, if any */
  platform(cl_platform_id platform id,
           int &error_code) {}
#endif

  /** Destructor of the SYCL abstraction */
  ~platform() {}


#ifdef TRISYCL_OPENCL
  /** Get the OpenCL platform_id underneath

      \todo Add cl.hpp version to the specification
  */
  cl_platform_id get() { assert(0); }
#endif


  /** Get the list of all the platforms available to the application */
  static VECTOR_CLASS<platform> get_platforms() { assert(0); }


#ifdef TRISYCL_OPENCL
  /** Get all the devices of a given type available to the application.

      By default returns all the devices.
  */
  static VECTOR_CLASS<device>
  get_devices(cl_device_type device_type = CL_DEVICE_TYPE_ALL) {
    assert(0);
  }


  /** Get the OpenCL information about the requested parameter

      \todo It looks like in the specification the cl::detail:: is lacking
      to fit the cl.hpp version. Or is it to be redefined in SYCL too?
  */
  template<cl_int name> typename
  cl::detail::param_traits<cl_platform_info, name>::param_type
  get_info() {
    assert(0);
  }
#endif


  /** Test if this platform is a host platform */
  bool is_host() {
    // Right now, this is a host-only implementation :-)
    return true;
  }


  /** Test if an extension is available on the platform

      \todo Should it be a param type instead of a STRING?

      \todo extend to any type of C++-string like object
  */
  bool has_extension(const STRING_CLASS extension_name) {
    assert(0);
  }

};


/** SYCL command group gather all the commands needed to execute one or
    more kernels in a kind of atomic way. Since all the parameters are
    captured at command group creation, one can execute the content in an
    asynchronous way and delayed schedule.

    For now just execute the command group directly.
 */
struct command_group {
  template <typename Functor>
  command_group(queue Q, Functor F) {
    F();
  }
};

/// @} to end the execution Doxygen group


/** \addtogroup data
    @{
*/

/** The accessor abstracts the way buffer data are accessed inside a
    kernel in a multidimensional variable length array way.

    \todo Implement it for images according so section 3.3.4.5
*/
template <typename dataType,
          std::size_t dimensions,
          access::mode mode,
          access::target target = access::global_buffer>
struct accessor
TRISYCL_IMPL(: AccessorImpl<dataType, dimensions, mode, target>) {
  /// \todo in the specification: store the dimension for user request
  static const auto dimensionality = dimensions;
  /// \todo in the specification: store the types for user request as STL
  // or C++AMP
  using element = dataType;
  using value_type = dataType;

#ifndef TRISYCL_HIDE_IMPLEMENTATION
  // Use a short-cut to the implementation because type name becomes quite
  // long...
  using Impl = AccessorImpl<dataType, dimensions, mode, target>;
#endif

  /// Create an accessor to the given buffer
  // \todo fix the specification to rename target that shadows template parm
  accessor(buffer<dataType, dimensions> &targetBuffer) :
    Impl(targetBuffer) {}

};


/** Abstract the way storage is managed to allow the programmer to control
    the storage management of buffers

    \param T
    the type of the elements of the underlying data

    The user is responsible for ensuring that their storage class
    implementation is thread-safe.
*/
template <typename T>
struct storage {
  /// \todo Extension to SYCL specification: provide pieces of STL
  /// container interface?
  using element = T;
  using value_type = T;


  /** Method called by SYCL system to get the number of elements of type T
      of the underlying data

      \todo This is inconsistent in the specification with get_size() in
      buffer which returns the byte size. Is it to be renamed to
      get_count()?
  */
  virtual std::size_t get_size() = 0;


  /** Method called by the SYCL system to know where that data is held in
      host memory

      \return the address or nullptr if SYCL has to manage the temporary
      storage of the data.
  */
  virtual T* get_host_data() = 0;


  /** Method called by the SYCL system at the point of construction to
      request the initial contents of the buffer

      \return the address of the data to use or nullptr to skip this data
      initialization
  */
  virtual const T* get_initial_data() = 0;


  /** Method called at the point of construction to request where the
      content of the buffer should be finally stored to

      \return the address of where the buffer will be written to in host
      memory.

      If the address is nullptr, then this phase is skipped.

      If get_host_data() returns the same pointer as get_initial_data()
      and/or get_final_data() then the SYCL system should determine whether
      copying is actually necessary or not.
  */
  virtual T* get_final_data() = 0;


  /** Method called when the associated memory object is destroyed.

      This method is only called once, so if a memory object is copied
      multiple times, only when the last copy of the memory object is
      destroyed is the destroy method called.

      Exceptions thrown by the destroy method will be caught and ignored.
  */
  virtual void destroy() = 0;


  /** \brief Method called when a command_group which accesses the data is
      added to a queue

     After completed is called, there may be further calls of
      in_use() if new work is enqueued that operates on the memory object.
  */
  virtual void in_use() = 0;


  /// Method called when the final enqueued command has completed
  virtual void completed() = 0;
};


/** A SYCL buffer is a multidimensional variable length array (à la C99
    VLA or even Fortran before) that is used to store data to work on.

    In the case we initialize it from a pointer, for now we just wrap the
    data with boost::multi_array_ref to provide the VLA semantics without
    any storage.

    \todo there is a naming inconsistency in the specification between
    buffer and accessor on T versus datatype
*/
template <typename T,
          std::size_t dimensions = 1>
struct buffer TRISYCL_IMPL(: BufferImpl<T, dimensions>) {
  /// \todo Extension to SYCL specification: provide pieces of STL
  /// container interface?
  using element = T;
  using value_type = T;

#ifndef TRISYCL_HIDE_IMPLEMENTATION
  // Use a short-cut because type name becomes quite long...
  using Impl = BufferImpl<T, dimensions>;
#endif

  /** Create a new buffer with storage managed by SYCL

      \param r defines the size
  */
  buffer(const range<dimensions> &r) : Impl(r) {}


  /** Create a new buffer with associated host memory

      \param host_data points to the storage and values used by the buffer

      \param r defines the size
  */
  buffer(T * host_data, range<dimensions> r) : Impl(host_data, r) {}


  /** Create a new read-only buffer with associated host memory

      \param host_data points to the storage and values used by the buffer

      \param r defines the size
  */
  buffer(const T * host_data, range<dimensions> r) :
    Impl(host_data, r) {}


  /** Create a new buffer from a storage abstraction provided by the user

      \param store is the storage back-end to use for the buffer

      \param r defines the size

      The storage object has to exist during all the life of the buffer
      object.

      \todo To be implemented
  */
  buffer(storage<T> &store, range<dimensions> r) { assert(0); }


  /** Create a new allocated 1D buffer initialized from the given elements

      \param start_iterator points to the first element to copy

      \param end_iterator points to just after the last element to copy

      \todo Add const to the SYCL specification
  */
  buffer(const T * start_iterator, const T * end_iterator) :
    Impl(start_iterator, end_iterator) {}


  /** Create a new buffer copy that shares the data with the origin buffer

      \param b is the buffer to copy from

      The system use reference counting to deal with data lifetime
  */
  buffer(buffer<T, dimensions> &b) : Impl(b) {}


  /** Create a new sub-buffer without allocation to have separate accessors
      later

      \param b is the buffer with the real data

      \param base_index specifies the origin of the sub-buffer inside the
      buffer b

      \param sub_range specifies the size of the sub-buffer

      \todo To be implemented

      \todo Update the specification to replace index by id
  */
  buffer(buffer<T, dimensions> b,
         id<dimensions> base_index,
         range<dimensions> sub_range) { assert(0); }


#ifdef TRISYCL_OPENCL
  /** Create a buffer from an existing OpenCL memory object associated to
      a context after waiting for an event signaling the availability of
      the OpenCL data

      \param mem_object is the OpenCL memory object to use

      \param from_queue is the queue associated to the memory object

      \param available_event specifies the event to wait for if non null

      \todo To be implemented

      \todo Improve the specification to allow CLHPP objects too
  */
  buffer(cl_mem mem_object,
         queue from_queue,
         event available_event) { assert(0); }
#endif


  // Use BOOST_DISABLE_ASSERTS at some time to disable range checking

  /** Get an accessor to the buffer with the required mode

      \param mode is the requested access mode

      \param target is the type of object to be accessed
  */
  template <access::mode mode,
            access::target target=access::global_buffer>
  accessor<T, dimensions, mode, target> get_access() {
    return { *this };
  }

};

/// @} to end the data Doxygen group

}
}

// Include the end of the implementation details
#include "implementation/sycl-implementation-end.hpp"

namespace cl {
namespace sycl {

/** \addtogroup parallelism
    @{
*/

/** kernel_lambda specify a kernel to be launch with a single_task or
    parallel_for

    \todo This seems to have also the kernel_functor name in the
    specification
*/
template <typename KernelName, typename Functor>
Functor kernel_lambda(Functor F) {
  return F;
}


/** SYCL single_task launches a computation without parallelism at launch
    time.

    Right now the implementation does nothing else that forwarding the
    execution of the given functor

    \todo remove from the SYCL specification and use a range-less
    parallel_for version with default construction of a 1-element range?
*/
void single_task(std::function<void(void)> F) { F(); }


#ifndef TRISYCL_HIDE_IMPLEMENTATION
/** SYCL parallel_for launches a data parallel computation with parallelism
    specified at launch time by a range<>

    \param global_size is the full size of the range<>

    \param f is the kernel functor to execute

    Unfortunately, to have implicit conversion to work on the range, the
    function can not be templated, so instantiate it for all the
    dimensions
*/
#define TRISYCL_ParallelForFunctor_GLOBAL(N)                          \
  template <typename ParallelForFunctor>                              \
  void parallel_for(range<N> global_size,                             \
                    ParallelForFunctor f) {                           \
    ParallelForImpl(global_size, f);                                  \
  }
TRISYCL_ParallelForFunctor_GLOBAL(1)
TRISYCL_ParallelForFunctor_GLOBAL(2)
TRISYCL_ParallelForFunctor_GLOBAL(3)
#endif


/** A variation of SYCL parallel_for to take into account a nd_range<>
 */
template <std::size_t Dimensions, typename ParallelForFunctor>
void parallel_for(nd_range<Dimensions> r, ParallelForFunctor f) {
  ParallelForImpl(r, f);
}


#ifndef TRISYCL_HIDE_IMPLEMENTATION
/** SYCL parallel_for launches a data parallel computation with
    parallelism specified at launch time by 1 range<> and an offset

    \param global_size is the global size of the range<>

    \param offset is the offset to be add to the id<> during iteration

    \param f is the kernel functor to execute

    Unfortunately, to have implicit conversion to work on the range, the
    function can not be templated, so instantiate it for all the
    dimensions
*/
#define TRISYCL_ParallelForFunctor_GLOBAL_OFFSET(N)                   \
  template <typename ParallelForFunctor>                              \
  void parallel_for(range<N> global_size,                             \
                    id<N> offset,                                     \
                    ParallelForFunctor f) {                           \
    ParallelForGlobalOffset(global_size, offset, f);  \
  }
TRISYCL_ParallelForFunctor_GLOBAL_OFFSET(1)
TRISYCL_ParallelForFunctor_GLOBAL_OFFSET(2)
TRISYCL_ParallelForFunctor_GLOBAL_OFFSET(3)
#endif


/// SYCL parallel_for version that allows a Program object to be specified
/* template <typename Range, typename Program, typename ParallelForFunctor>
void parallel_for(Range r, Program p, ParallelForFunctor f) {
  /// \todo deal with Program
  parallel_for(r, f);
}
*/

/// Loop on the work-groups
template <std::size_t Dimensions = 1, typename ParallelForFunctor>
void parallel_for_workgroup(nd_range<Dimensions> r,
                            ParallelForFunctor f) {
  ParallelForWorkgroup(r, f);
}


/// Loop on the work-items inside a work-group
template <std::size_t Dimensions = 1, typename ParallelForFunctor>
void parallel_for_workitem(group<Dimensions> g, ParallelForFunctor f) {
  ParallelForWorkitem(g, f);
}

/// @} End the parallelism Doxygen group


/** \addtogroup address_spaces
    @{
*/

/** Declare a variable to be an OpenCL constant pointer

    \param T is the pointer type

    Note that if \a T is not a pointer type, it is an error.
*/
#ifdef TRISYCL_HIDE_IMPLEMENTATION
template <typename T>
struct constant {
  // This is only for Doxygen documentation for SYCL API
};
#else
template <typename T>
using constant = AddressSpaceImpl<T, constant_address_space>;
#endif


/** Declare a variable to be an OpenCL 2 generic pointer

    \param T is the pointer type

    Note that if \a T is not a pointer type, it is an error.
*/
#ifdef TRISYCL_HIDE_IMPLEMENTATION
template <typename T>
struct generic {
  // This is only for Doxygen documentation for SYCL API
};
#else
template <typename T>
using generic = AddressSpaceImpl<T, generic_address_space>;
#endif


/** Declare a variable to be an OpenCL global pointer

    \param T is the pointer type

    Note that if \a T is not a pointer type, it is an error.
*/
#ifdef TRISYCL_HIDE_IMPLEMENTATION
template <typename T>
struct global {
  // This is only for Doxygen documentation for SYCL API
};
#else
template <typename T>
using global = AddressSpaceImpl<T, global_address_space>;
#endif


/** Declare a variable to be an OpenCL local pointer

    \param T is the pointer type

    Note that if \a T is not a pointer type, it is an error.
*/
#ifdef TRISYCL_HIDE_IMPLEMENTATION
template <typename T>
struct local {
  // This is only for Doxygen documentation for SYCL API
};
#else
template <typename T>
using local = AddressSpaceImpl<T, local_address_space>;
#endif


/** Declare a variable to be an OpenCL private pointer

    \param T is the pointer type

    Note that if \a T is not a pointer type, it is an error.
*/
#ifdef TRISYCL_HIDE_IMPLEMENTATION
template <typename T>
struct priv {
  // This is only for Doxygen documentation for SYCL API
};
#else
template <typename T>
using priv = AddressSpaceImpl<T, private_address_space>;
#endif


/** A pointer that can be statically associated to any address-space

    \param Pointer is the pointer type

    \param AS is the address space to point to

    Note that if \a Pointer is not a pointer type, it is an error.
*/
#ifdef TRISYCL_HIDE_IMPLEMENTATION
template <typename Pointer, address_space AS>
struct multi_ptr {
  // This is only for Doxygen documentation for SYCL API
};
#else
template <typename Pointer, address_space AS>
using multi_ptr = AddressSpacePointerImpl<Pointer, AS>;
#endif


/** Construct a cl::sycl::multi_ptr<> with the right type

    \param pointer is the address with its address space to point to

    \todo Implement the case with a plain pointer
*/
template <typename T, address_space AS>
multi_ptr<T, AS> make_multi(multi_ptr<T, AS> pointer) {
  return pointer;
}


/// @} End the address_spaces Doxygen group

}
}


/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/
