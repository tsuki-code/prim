#ifndef __MAYBE_HPP__
#define __MAYBE_HPP__

#include <exception>

#include "../poly/value.hpp"
#include "../poly/vtable/vtable.hpp"
#include "../poly/allocator.hpp"

namespace impl__{ struct Empty_maybe{  }; }

class empty_maybe_access :
  public std::exception
{
public:
  virtual const char* what() const noexcept override
  { return "Attempted access to empty Maybe"; }
};

template< typename T__ >
class Maybe{
private:
  using Empty = impl__::Empty_maybe;
  using T = std::conditional_t<
    std::is_reference<T__>::value,
    std::reference_wrapper< std::decay_t<T__> >,
    std::decay_t<T__>
    >;

  struct Get :
    poly::Invoker< Get,
		   T& (poly::T& ),
		   const T& (const poly::T&) >
  {  };

  friend T& invoke( Get, T& value ){ return value; }
  friend T& invoke( Get, Empty& )  { throw empty_maybe_access(); }
  
  struct Interface
    : poly::Interface< Get, poly::copy, poly::move, poly::destroy, poly::storage >
  {  };
  
  using Value_t = poly::Value< poly::JumpVT< Interface, T, Empty >,
			       poly::StackAllocator< sizeof(T), alignof(T) > >;
public:
  Maybe()
    : value_{ in_place<Empty>() }{  }
  Maybe( const T& value )
    : value_( in_place<T>(), value ){  }
  Maybe( T&& value )
    : value_( in_place<T>(), std::move(value) )
  {  }

  const T& get() const { return Get::call( value_ ); }
        T& get()       { return Get::call( value_ ); }

  template< typename... Args >
  void emplace( Args&&... args ){ value_.template emplace<T>( std::forward<Args>(args)... ); }
  void reset(){ *this = {}; }
	    
  bool empty(){ return value_.vtable().index() == 1; }

private:
  Value_t value_;
};

#endif //__MAYBE_HPP__