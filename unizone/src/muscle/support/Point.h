/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

/*******************************************************************************
/
/   File:      Point.h
/
/   Description:     version of Be's Point class
/
*******************************************************************************/

#ifndef MusclePoint_h
#define MusclePoint_h

#include "support/Flattenable.h"
#include "support/Tuple.h"

namespace muscle {

/*----------------------------------------------------------------------*/
/*----- Point class --------------------------------------------*/

/** A portable version of Be's BPoint class. */
class Point : public Flattenable, public Tuple<2,float>
{
public:
   /** Default constructor, sets the point to be (0.0f, 0.0f) */
   Point() {/* empty */}

   /** Constructor where you specify the initial value of the point 
    *  @param ax Initial x position
    *  @param ay Initial y position
    */
   Point(float ax, float ay) {Set(ax, ay);}

   /** Copy Constructor. */
   Point(const Point & rhs) : Flattenable(), Tuple<2,float>(rhs) {/* empty */}

   /** Destructor */
   virtual ~Point() {/* empty */}
 
   /** convenience method to set the x value of this Point */
   inline float & x()       {return (*this)[0];}

   /** convenience method to get the x value of this Point */
   inline float   x() const {return (*this)[0];}

   /** convenience method to set the y value of this Point */
   inline float & y()       {return (*this)[1];}

   /** convenience method to get the y value of this Point */
   inline float   y() const {return (*this)[1];}

   /** Sets a new value for the point.
    *  @param ax The new x value 
    *  @param ay The new y value 
    */
   void Set(float ax, float ay) {x() = ax; y() = ay;}

   /** If the point is outside the rectangle specified by the two arguments,
    *  it will be moved horizontally and/or vertically until it falls inside the rectangle.
    *  @param topLeft Minimum values acceptable for x and y
    *  @param bottomRight Maximum values acceptable for x and y
    */
   void ConstrainTo(Point topLeft, Point bottomRight)
   {
      if (x() < topLeft.x())     x() = topLeft.x();
      if (y() < topLeft.y())     y() = topLeft.y();
      if (x() > bottomRight.x()) x() = bottomRight.x();
      if (y() > bottomRight.y()) y() = bottomRight.y();
   }

   /** Print debug information about the point to stdout */
   void PrintToStream() const
   {
      printf("Point: %f %f\n", x(), y());
   }
      
   /** Part of the Flattenable interface:  Returns true */
   virtual bool IsFixedSize() const {return true;} 

   /** Part of the Flattenable interface:  Returns B_POINT_TYPE */
   virtual type_code TypeCode() const {return B_POINT_TYPE;}

   /** Part of the Flattenable interface:  2*sizeof(float) */
   virtual uint32 FlattenedSize() const {return 2*sizeof(float);}

   /** Copies this point into an endian-neutral flattened buffer.
    *  @param buffer Points to an array of at least FlattenedSize() bytes.
    */
   virtual void Flatten(uint8 * buffer) const 
   {
      float * buf = (float *) buffer;
      float ox = B_HOST_TO_LENDIAN_FLOAT(x()); muscleCopyOut(&buf[0], ox);
      float oy = B_HOST_TO_LENDIAN_FLOAT(y()); muscleCopyOut(&buf[1], oy);
   }

   /** Restores this point from an endian-neutral flattened buffer.
    *  @param buffer Points to an array of (size) bytes
    *  @param size The number of bytes (buffer) points to (should be at least FlattenedSize())
    *  @return B_NO_ERROR on success, B_ERROR on failure (size was too small)
    */
   virtual status_t Unflatten(const uint8 * buffer, uint32 size) 
   {
      if (size >= FlattenedSize())
      {
         float * buf = (float *) buffer;
         muscleCopyIn(x(), &buf[0]); x() = B_LENDIAN_TO_HOST_FLOAT(x());
         muscleCopyIn(y(), &buf[1]); y() = B_LENDIAN_TO_HOST_FLOAT(y());
         return B_NO_ERROR;
      }
      else return B_ERROR;
   }
};

DECLARE_ALL_TUPLE_OPERATORS(Point,float);

};  // end namespace muscle

#endif 
