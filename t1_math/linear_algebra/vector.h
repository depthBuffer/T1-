#ifndef MATH_LINEAR_ALGEBRA_VECTOR_H
#define MATH_LINEAR_ALGEBRA_VECTOR_H

#include <initializer_list>
#include <assert.h>
#include <array>

namespace math
{
    /* 
      Generic vector template version supporting vectors of any type and any
      positive dimension. Constructors and templated vector mathematics are 
      defined outside the templated class to support shared functionality in
      combination with template specialization of vec2,vec3 and vec4.

      The underlying implementation of all the vector types are based on C++
      templates. The most common found vector classes are however defined with
      preprocessor types for better readability hiding the underlying template
      implementation. For vectors of a dimension > 4 it is required to directly
      specify the template variables.

    */
    template <std::size_t n, class T>
    struct vector
    {
    public:
		std::array<T, n> data;

        // constructor0: default empty constructor; default initialize all vector elements
        vector()
        {
            // the empty constructor frequently gets called (when declaring large lists of
            // vectors for instance) so default initializing them can be expensive as in almost all 
            // cases we'll be setting them to different values anyways.
            /*for (unsigned int i = 0; i < n; ++i)
                e[i] = {};*/
        }
        // constructor1: one argument given: initialize all vectors elements w/ same value
        vector(const T& v)
        {
			for (auto& el : data) {
				el = v;
			}
        }
        // constructor2: use std::initializer list for accepting any number of arguments
        vector(const std::initializer_list<T> args)
        {
            assert(args.size() < n);
			data = args;
        }

        // subscript operator
        T& operator[] (const std::size_t index)
        {
            assert(index >= 0 && index < n);
            return data.at(index);
        }

        // math member operators (defined in operation.h)
        // ---------------------------------------------------
        // negate operator
        vector<n, T> operator-();
    };

    /* 

      Specialized vector version for 2-dimensional vectors.

    */
    template <typename T>
    struct vector<2, T>
    {
        union 
        {
			std::array<T, 2> data;
            // position
            struct 
            {
                T x;
                T y;
            };
            // texture coordinates
            // only available in vec2, due to .w being reserved for position.
            struct
            {
                T u;
                T v;
            };
            // texture coordinates
            // s,t,r is the general form of accessing texture coordinates in OpenGL.
            // Using u,v,w in 3 dimensions requires w which is reserved for the position's 
            // homogenous coordinate set.
            struct
            {
                T s;
                T t;
            };
        };

        // constructor0: default empty constructor; default initialize all vector elements
        vector()
        {
			data = { };
        }
        // constructor1: one argument given: initialize all vectors elements w/ same value
        vector(const T& v)
        {
			data = { v, v };
        }
        // constructor2: use std::initializer list for accepting any number of arguments
        vector(const std::initializer_list<T> args)
        {
            assert(args.size() <= 2);
            int index = 0;
			for (auto begin = args.begin(); begin != args.end(); ++begin) {
				data.at(index++) = *begin;
			}
        }
        // constructor3: vec2 per-element initialization
        vector(const T& x, const T& y)
        {
			data = { x, y };
        }

        // subscript operator
        T& operator[] (const std::size_t index)
        {
            assert(index >= 0 && index < 2);
            return data.at(index);
        }

        // math operators (defined in operation.h)
        // ---------------------------------------------------
        // negate operator
        vector<2, T> operator-();
    };

    /* 
        
       Specialized vector version for 3-dimensional vectors.

    */
    template<typename T>
    struct vector<3, T>
    {
        union 
        {
			std::array<T, 3> data;
            // position
            struct
            {
                T x;
                T y;
                T z;
            };
            // color
            struct
            {
                T r;
                T g;
                T b;
            };
            // texture coordinates
            struct
            {
                T s;
                T t;
                T _r;
            };
            struct
            {
                vector<2, T> xy;
            };
            struct
            {
                vector<2, T> yz;
            };
        };

        // static frequently used vectors
        static vector<3, T> UP;
        static vector<3, T> DOWN;
        static vector<3, T> LEFT;
        static vector<3, T> RIGHT;
        static vector<3, T> FORWARD;
        static vector<3, T> BACK;

        // constructor0: default empty constructor; default initialize all vector elements
        vector()
        {
			data = {};
        }
        // constructor1: one argument given: initialize all vectors elements w/ same value
        vector(const T& v)
        {
			data = { v, v, v };
        }
        // constructor2: use std::initializer list for accepting any number of arguments
        vector(const std::initializer_list<T> args)
        {
            assert(args.size() <= 3);
            int index = 0;
			for (auto begin = args.begin(); begin != args.end(); ++begin) {
				data.at(index++) = *begin;
			}
        }
        // constructor3: vec3 per-element initialization
        vector(const T& x, const T& y, const T& z)
        {
			data = { x, y, z };
        }
        // constructor4: vec2 initialization
        vector(const vector<2, T>& vec, const T& z)
        {
			data = { vec.x, vec.y, z };
        }
        // constructor5: vec2 initialization
        vector(const T& x, const vector<2, T>& vec)
        {
			data = { x, vec.x, vec.y };
        }

        // subscript operator
        T& operator[] (const std::size_t index)
        {
            assert(index >= 0 && index < 3);
            return data.at(index);
        }

        // math operators (defined in operation.h)
        // ---------------------------------------------------
        // negate operator
        vector<3, T> operator-();
    };

    // initialize static variables of vec3
    template<typename T> vector<3, T> vector<3, T>::UP      = vector<3, T>( 0.0,  1.0,  0.0);
    template<typename T> vector<3, T> vector<3, T>::DOWN    = vector<3, T>( 0.0, -1.0,  0.0);
    template<typename T> vector<3, T> vector<3, T>::LEFT    = vector<3, T>(-1.0,  0.0,  0.0);
    template<typename T> vector<3, T> vector<3, T>::RIGHT   = vector<3, T>( 1.0,  0.0,  0.0);
    template<typename T> vector<3, T> vector<3, T>::FORWARD = vector<3, T>( 0.0,  0.0, -1.0);
    template<typename T> vector<3, T> vector<3, T>::BACK    = vector<3, T>( 0.0,  0.0,  1.0);

    /* 

    Specialized vector version for 4-dimensional vectors.

    */
    template<typename T>
    struct vector<4, T>
    {
        union
        {
			std::array<T, 4> data;
            // position
            struct
            {
                T x;
                T y;
                T z;
                T w;
            };
            // color
            struct
            {
                T r;
                T g;
                T b;
                T a;
            };
            // texture coordinates
            struct
            {
                T s;
                T t;
                T _r;
            };
            struct
            {
                vector<2, T> xy;
                T _ignored1;
                T _ignored2;
            };
            struct
            {
                T __ignored1;
                T __ignored2;
                vector<2, T> yz;
            };
            struct
            {
                vector<3, T> xyz;
                T _ignored1_;
            };
            struct
            {
                vector<3, T> rgb;
                T __ignored1_;
            };
            struct
            {
                vector<3, T> srt;
                T __ignored1__;
            };
        };

        // constructor0: default empty constructor; default initialize all vector elements
        vector()
        {
			data = { };
        }
        // constructor1: one argument given: initialize all vectors elements w/ same value
        vector(const T& v)
        {
			data = { v, v, v, v };
        }
        // constructor2: use std::initializer list for accepting any number of arguments
        vector(const std::initializer_list<T> args)
        {
            assert(args.size() <= 4);
            int index = 0;
			for (auto begin = args.begin(); begin != args.end(); ++begin) {
				data.at(index++) = *begin;
			}
        }
        // constructor3: vec3 per-element initialization
        vector(const T& x, const T& y, const T& z, const T& w)
        {
			data = { x, y, z, w };
        }
        // constructor4: vec2 initialization
        vector(const vector<2, T>& xy, const vector<2, T>& zw)
        {
			data = { xy.x, xy.y, zw.z, zw.w };
        }
        // constructor5: vec3 initialization
        vector(const vector<3, T>& xyz, const T& w)
        {
			data = { xyz.x, xyz.y, xyz.z, w };
        }

        // subscript operator
        T& operator[] (const std::size_t index)
        {
            assert(index >= 0 && index < 4);
            return data.at(index);
        }     

        // math operators (defined in operation.h)
        // ---------------------------------------------------
        // negate operator
        vector<4, T> operator-();
    };

    typedef vector<2, float>  vec2;
    typedef vector<3, float>  vec3;
    typedef vector<4, float>  vec4;
    typedef vector<2, int>    ivec2;
    typedef vector<3, int>    ivec3;
    typedef vector<4, int>    ivec4;
    typedef vector<2, double> dvec2;
    typedef vector<3, double> dvec3;
    typedef vector<4, double> dvec4;

    // per-vector operations
    // ---------------------------------
    // negate operator
    // because this is a member operator, we have to
    // define this for each specialization.
    template <std::size_t n, typename T>
    inline vector<n, T> vector<n, T>::operator-()
    {
        vector<n, T> result;
		for (std::size_t i = 0; i < n; ++i) {
			result[i] = -data[i];
		}
        return result;
    }
    template <typename T>
    inline vector<2, T> vector<2, T>::operator-()
    {
        return{ -x, -y };
    }
    template <typename T>
    inline vector<3, T> vector<3, T>::operator-()
    {
        return{ -x, -y, -z };
    }
    template <typename T>
    inline vector<4, T> vector<4, T>::operator-()
    {
        return{ -x, -y, -z, -w };
    }

    // addition
    template <std::size_t n, typename T>
    inline vector<n, T> operator+(vector<n, T> lhs, T scalar)
    {
        vector<n, T> result;
		for (std::size_t i = 0; i < n; ++i) {
			result[i] = lhs[i] + scalar;
		}
        return result;
    }
    template <std::size_t n, typename T>
    inline vector<n, T> operator+(T scalar, vector<n, T> rhs)
    {
        vector<n, T> result;
        for (std::size_t i = 0; i < n; ++i)
            result[i] = rhs[i] + scalar; //lhs[i] + scalar; something wrong
        return result;
    }
    template <std::size_t n, typename T>
    inline vector<n, T> operator+(vector<n, T> lhs, vector<n, T> rhs)
    {
        vector<n, T> result;
        for (std::size_t i = 0; i < n; ++i)
            result[i] = lhs[i] + rhs[i];
        return result;
    }

    // subtraction
    template <std::size_t n, typename T>
    inline vector<n, T> operator-(vector<n, T> lhs, T scalar)
    {
        vector<n, T> result;
		for (std::size_t i = 0; i < n; ++i) {
			result[i] = lhs[i] - scalar;
		}
        return result;
    }
    template <std::size_t n, typename T>
    inline vector<n, T> operator-(vector<n, T> lhs, vector<n, T> rhs)
    {
        vector<n, T> result;
		for (std::size_t i = 0; i < n; ++i) {
			result[i] = lhs[i] - rhs[i];
		}
        return result;
    }

    // multiplication
    template <std::size_t n, typename T>
    inline vector<n, T> operator*(vector<n, T> lhs, T scalar)
    {
        vector<n, T> result;
		for (std::size_t i = 0; i < n; ++i) {
			result[i] = lhs[i] * scalar;
		}
        return result;
    }
    template <std::size_t n, typename T>
    vector<n, T>& operator*=(vector<n, T>& lhs, T scalar)
    {
        for (std::size_t i = 0; i < n; ++i) {
            lhs[i] *= scalar;
        }
        return lhs;
    }
    template <std::size_t n, typename T>
    inline vector<n, T> operator*(T scalar, vector<n, T> lhs)
    {
        vector<n, T> result;
		for (std::size_t i = 0; i < n; ++i) {
			result[i] = lhs[i] * scalar;
		}
        return result;
    }
    template <std::size_t n, typename T>
    inline vector<n, T> operator*(vector<n, T> lhs, vector<n, T> rhs) // hadamard product
    {
        vector<n, T> result;
		for (std::size_t i = 0; i < n; ++i) {
			result[i] = lhs[i] * rhs[i];
		}
        return result;
    }

    // division
    template <std::size_t n, typename T>
    inline vector<n, T> operator/(vector<n, T> lhs, T scalar)
    {
        vector<n, T> result;
		for (unsigned int i = 0; i < n; ++i) {
			result[i] = lhs[i] / scalar;
		}
        return result;
    }
    template <std::size_t n, typename T>
    inline vector<n, T> operator/(T scalar, vector<n, T> lhs)
    {
        vector<n, T> result;
		for (std::size_t i = 0; i < n; ++i) {
			result[i] = lhs[i] / scalar;
		}
        return result;
    }
    template <std::size_t n, typename T>
    inline vector<n, T> operator/(vector<n, T> lhs, vector<n, T> rhs) // hadamard product
    {
        vector<n, T> result;
		for (std::size_t i = 0; i < n; ++i) {
			result[i] = lhs[i] / rhs[i];
		}
        return result;
    }
} // namespace math

#endif