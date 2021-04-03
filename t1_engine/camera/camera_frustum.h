#ifndef T1_CAMERA_FRUSTUM_H
#define T1_CAMERA_FRUSTUM_H

#include <t1_math/linear_algebra/vector.h>
#include <t1_math/linear_algebra/operation.h>

namespace T1
{
    class Camera;

    /* 
        
      Data container for the 3D plane equation variables in Cartesian space. A plane equation can be
      defined by its Normal (perpendicular to the plane) and a distance value D obtained from any
      point on the plane itself (projected onto the normal vector).

    */
    struct FrustumPlane
    {
        math::vec3 Normal;
        float      D;

        void SetNormalD(math::vec3 normal, math::vec3 point)
        {
            Normal = math::normalize(normal);
            D      = -math::dot(Normal, point);
        }

        float Distance(math::vec3 point)
        {
            return math::dot(Normal, point) + D;
        }
    };


    /*

      Container object managing all 6 camera frustum planes as calculated from any Camera object.

      The CameraFrustum allows to quickly determine (using simple collision primitives like point,
      sphere, box) whether an object is within the frustum (or crossing the frustum's edge(s)). 
      This gives us the option to significantly reduce draw calls for objects that aren't visible
      anyways. Note that the frustum needs to be re-calculated every frame.

    */
    class CameraFrustum
    {
    public:
        FrustumPlane Planes[6];

        // named access to the individual planes (an anonymous struct inside a union would be
        // shorter, but standard C++ doesn't allow members with constructors in one; only MSVC does)
        FrustumPlane& Left()   { return Planes[0]; }
        FrustumPlane& Right()  { return Planes[1]; }
        FrustumPlane& Top()    { return Planes[2]; }
        FrustumPlane& Bottom() { return Planes[3]; }
        FrustumPlane& Near()   { return Planes[4]; }
        FrustumPlane& Far()    { return Planes[5]; }

    public:
        CameraFrustum() { }

        void Update(Camera* camera);

        bool Intersect(math::vec3 point);
        bool Intersect(math::vec3 point, float radius);
        bool Intersect(math::vec3 boxMin, math::vec3 boxMax);
    };
}
#endif