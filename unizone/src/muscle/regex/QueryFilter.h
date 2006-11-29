/* This file is Copyright 2007 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef MuscleQueryFilter_h
#define MuscleQueryFilter_h

#include "util/Queue.h"
#include "util/ByteBuffer.h"
#include "message/Message.h"

BEGIN_NAMESPACE(muscle);

class DataNode;
class StringMatcher;

/** Enumeration of QueryFilter type codes for the included QueryFilter classes */
enum {
   QUERY_FILTER_TYPE_WHATCODE = 1902537776, // 'qfl0'
   QUERY_FILTER_TYPE_VALUEEXISTS,
   QUERY_FILTER_TYPE_BOOL,
   QUERY_FILTER_TYPE_DOUBLE,
   QUERY_FILTER_TYPE_FLOAT,
   QUERY_FILTER_TYPE_INT64,
   QUERY_FILTER_TYPE_INT32,
   QUERY_FILTER_TYPE_INT16,
   QUERY_FILTER_TYPE_INT8,
   QUERY_FILTER_TYPE_POINT,
   QUERY_FILTER_TYPE_RECT,
   QUERY_FILTER_TYPE_STRING,
   QUERY_FILTER_TYPE_MESSAGE,
   QUERY_FILTER_TYPE_RAWDATA,
   QUERY_FILTER_TYPE_NANDNOT,
   QUERY_FILTER_TYPE_ANDOR,
   QUERY_FILTER_TYPE_XOR,
   // add more codes here...
   LAST_QUERY_FILTER_TYPE
};

/** Interface for any object that can examine a Message and tell whether it
  * matches some criterion.  Used primarily for filtering queries based on content.
  */
class QueryFilter : public RefCountable
{
public:
   /** Default constructor */
   QueryFilter() {/* empty */}

   /** Destructor */
   virtual ~QueryFilter();

   /** Dumps our state into the given (archive).   Default implementation
    *  just writes our TypeCode() into the 'what' of the Message
    *  and returns B_NO_ERROR.
    *  @param archive the Message to write our state into.
    *  @returns B_NO_ERROR on success, or B_ERROR on failure.
    */
   virtual status_t SaveToArchive(Message & archive) const;

   /** Restores our state from the given (archive).  Default implementation
    *  returns B_NO_ERROR iff the archive's what code matches our TypeCode().
    *  @param archive The archive to restore our state from.
    *  @returns B_NO_ERROR on success, or B_ERROR on failure.
    */     
   virtual status_t SetFromArchive(const Message & archive);

   /** Should be overridden to return the appropriate QUERY_FILTER_TYPE_* code. */
   virtual uint32 TypeCode() const = 0;

   /** Must be implemented to return true iff (msg) matches the criterion.
     * @param msg the Message to check
     * @param optNode The DataNode object the matching is being done on, or NULL if the DataNode is not available.
     * @returns true iff the Messages matches, else false.
     */
   virtual bool Matches(const Message & msg, const DataNode * optNode) const = 0;

   /** Returns true iff we can be instantiated using a Message with the given
     * 'what' code.  Default implementation returns true iff (what) equals the
     * value returned by our own TypeCode() method.
     */
   virtual bool AcceptsTypeCode(uint32 what) const {return TypeCode() == what;}
};

/** Type for a reference to a queue of StringMatcher objects. */
typedef Ref<QueryFilter> QueryFilterRef;

/** This filter tests the 'what' value of the Message. */
class WhatCodeQueryFilter : public QueryFilter
{
public:
   /** Default constructor
     * @param what The 'what' code to match on.  Only Messages with this 'what' code will be matched.
     */
   WhatCodeQueryFilter(uint32 what = 0) : _minWhatCode(what), _maxWhatCode(what) {/* empty */}

   /** Default constructor
     * @param minWhat The minimum 'what' code to match on.  Only messages with 'what' codes greater than or equal to this code will be matched.
     * @param maxWhat The maximum 'what' code to match on.  Only messages with 'what' codes less than or equal to this code will be matched.
     */
   WhatCodeQueryFilter(uint32 minWhat, uint32 maxWhat) : _minWhatCode(minWhat), _maxWhatCode(maxWhat) {/* empty */}

   virtual status_t SaveToArchive(Message & archive) const;
   virtual status_t SetFromArchive(const Message & archive);
   virtual bool Matches(const Message & msg, const DataNode *) const {return muscleInRange(msg.what, _minWhatCode, _maxWhatCode);}
   virtual uint32 TypeCode() const {return QUERY_FILTER_TYPE_WHATCODE;}

private:
   uint32 _minWhatCode;
   uint32 _maxWhatCode;
};

/** Semi-abstract base class for all query filters that test a single item in a Message */
class ValueQueryFilter : public QueryFilter
{
public:
   /** Default constructor */
   ValueQueryFilter() : _index(0) {/* Empty */}

   /** Constructor.
     * @param fieldName Name of the field in the Message to look at
     * @param index Index of the item within the field to look at.  Defaults to zero (i.e. the first item)
     */
   ValueQueryFilter(const String & fieldName, uint32 index = 0) : _fieldName(fieldName), _index(index) {/* empty */}

   virtual status_t SaveToArchive(Message & archive) const;
   virtual status_t SetFromArchive(const Message & archive);

   /** Sets our index-in-field setting. */
   void SetIndex(uint32 index) {_index = index;}

   /** Returns our current index-in-field setting, as set by SetIndex() or in our constructor */
   uint32 GetIndex() const {return _index;}

   /** Sets the field name to use. */
   void SetFieldName(const String & fieldName) {_fieldName = fieldName;}

   /** Returns the current field name, as set by SetFieldName() or in our constructor. */
   const String & GetFieldName() const {return _fieldName;}

private:
   String _fieldName;
   uint32 _index;
};

/** This filter merely checks to see if the specified value exists in the target Message. */
class ValueExistsQueryFilter : public ValueQueryFilter
{
public:
   /** Default constructor.  
     * @param typeCode Type code to check for.  Default to B_ANY_TYPE (a wildcard value)
     */
   ValueExistsQueryFilter(uint32 typeCode = B_ANY_TYPE) : _typeCode(typeCode) {/* empty */}

   /** Constructor
     * @param fieldName Field name to look for
     * @param typeCode type code to look for, or B_ANY_TYPE if you don't care what the type code is.
     * @param index Optional index of the item within the field.  Defaults to zero.
     */
   ValueExistsQueryFilter(const String & fieldName, uint32 typeCode = B_ANY_TYPE, uint32 index = 0) : ValueQueryFilter(fieldName, index), _typeCode(typeCode) {/* empty */}

   virtual status_t SaveToArchive(Message & archive) const;
   virtual status_t SetFromArchive(const Message & archive);
   virtual uint32 TypeCode() const {return QUERY_FILTER_TYPE_VALUEEXISTS;}
   virtual bool Matches(const Message & msg, const DataNode *) const {const void * junk; return (msg.FindData(GetFieldName(), _typeCode, &junk, NULL) == B_NO_ERROR);}

   /** Sets the type code that we will look for in the target Message.
     * @param typeCode the type code to look for.  Use B_ANY_TYPE to indicate that you don't care what the type code is.
     */
   void SetTypeCode(uint32 typeCode) {_typeCode = typeCode;}

   /** Returns the type code we are to look in the target Message for.
     * Note that this method is different from TypeCode()!
     */
   uint32 GetTypeCode() const {return _typeCode;}

private:
   uint32 _typeCode;
};

/** This templated class is used to generate a number of numeric-comparison-query classes, all of which are quite similar to each other. */
template <typename DataType, uint32 DataTypeCode, uint32 ClassTypeCode>
class NumericQueryFilter : public ValueQueryFilter
{
public:
   /** Default constructor.  Sets our value to its default (usually zero), and the operator to OP_EQUAL_TO. */
   NumericQueryFilter() : _value(), _op(OP_EQUAL_TO) {/* empty */}

   /** Constructor.
     * @param fieldName Field name to look under.
     * @param op The operator to use (should be one of the OP_* values enumerated below)
     * @param value The value to compare to the value found in the Message.
     * @param index Optional index of the item within the field.  Defaults to zero.
     */
   NumericQueryFilter(const String & fieldName, uint8 op, DataType value, uint32 index = 0) : ValueQueryFilter(fieldName, index), _value(value), _op(op) {/* empty */}

   virtual status_t SaveToArchive(Message & archive) const
   {
      return ((ValueQueryFilter::SaveToArchive(archive) == B_NO_ERROR)&&
              (archive.AddInt8("op", _op) == B_NO_ERROR)) ? archive.AddData("val", DataTypeCode, &_value, sizeof(_value)) : B_ERROR;
   }

   virtual status_t SetFromArchive(const Message & archive)
   {
      DataType * dt;
      uint32 numBytes;
      if ((ValueQueryFilter::SetFromArchive(archive) == B_NO_ERROR)&&
          (archive.FindInt8("op", (int8*)&_op)       == B_NO_ERROR)&&
          (archive.FindData("val", DataTypeCode, (const void **)&dt, &numBytes) == B_NO_ERROR)&&(numBytes == sizeof(_value)))
      {
         _value = *dt;
         return B_NO_ERROR;
      }
      return B_ERROR;
   }

   virtual uint32 TypeCode() const {return ClassTypeCode;}

   virtual bool Matches(const Message & msg, const DataNode *) const
   {
      bool ret = false;
      DataType * temp;
      if (msg.FindData(GetFieldName(), DataTypeCode, GetIndex(), (const void **) &temp, NULL) == B_NO_ERROR)
      {
         switch(_op)
         {
            case OP_EQUAL_TO:                 ret = (*temp == _value); break;
            case OP_LESS_THAN:                ret = (*temp <  _value); break;
            case OP_GREATER_THAN:             ret = (*temp >  _value); break;
            case OP_LESS_THAN_OR_EQUAL_TO:    ret = (*temp <= _value); break;
            case OP_GREATER_THAN_OR_EQUAL_TO: ret = (*temp >= _value); break;
            case OP_NOT_EQUAL_TO:             ret = (*temp != _value); break;
            default:                          /* do nothing */         break;
         }
      }
      return ret;
   }

   /** Set the operator to use.  
     * @param op One of the OP_* values enumerated below.
     */
   void SetOperator(uint8 op) {_op = op;}

   /** Returns the currently specified operator, as specified in the constructor or in SetOperator() */
   uint8 GetOperator() const {return _op;}

   /** Set the value to compare against.
     * @param value The new value.
     */
   void SetValue(DataType value) {_value = value;}

   /** Returns the currently specified value, as specified in the constructor or in SetValue() */
   DataType GetValue() const {return _value;}
 
   /** Operators defined for our expressions */
   enum {
      OP_EQUAL_TO = 0,
      OP_LESS_THAN,
      OP_GREATER_THAN,
      OP_LESS_THAN_OR_EQUAL_TO,
      OP_GREATER_THAN_OR_EQUAL_TO,
      OP_NOT_EQUAL_TO,
      NUM_NUMERIC_OPERATORS
   };

private:
   DataType _value;
   uint8 _op;
};

typedef NumericQueryFilter<bool,   B_BOOL_TYPE,   QUERY_FILTER_TYPE_BOOL>   BoolQueryFilter;
typedef NumericQueryFilter<double, B_DOUBLE_TYPE, QUERY_FILTER_TYPE_DOUBLE> DoubleQueryFilter;
typedef NumericQueryFilter<float,  B_FLOAT_TYPE,  QUERY_FILTER_TYPE_FLOAT>  FloatQueryFilter;
typedef NumericQueryFilter<int64,  B_INT64_TYPE,  QUERY_FILTER_TYPE_INT64>  Int64QueryFilter;
typedef NumericQueryFilter<int32,  B_INT32_TYPE,  QUERY_FILTER_TYPE_INT32>  Int32QueryFilter;
typedef NumericQueryFilter<int16,  B_INT16_TYPE,  QUERY_FILTER_TYPE_INT16>  Int16QueryFilter;
typedef NumericQueryFilter<int8,   B_INT8_TYPE,   QUERY_FILTER_TYPE_INT8>   Int8QueryFilter;
typedef NumericQueryFilter<Point,  B_POINT_TYPE,  QUERY_FILTER_TYPE_POINT>  PointQueryFilter;
typedef NumericQueryFilter<Point,  B_RECT_TYPE,   QUERY_FILTER_TYPE_RECT>   RectQueryFilter;

/** A semi-abstract base class for any QueryFilter that holds a list of references to child filters. */
class MultiQueryFilter : public QueryFilter
{
public:
   /** Default constructor. */
   MultiQueryFilter() {/* empty */}

   virtual status_t SaveToArchive(Message & archive) const;
   virtual status_t SetFromArchive(const Message & archive);

   /** Returns a read-only reference to our Queue of child QueryFilterRefs. */
   const Queue<QueryFilterRef> & GetChildren() const {return _children;}

   /** Returns a read/write reference to our Queue of child QueryFilterRefs. */
   Queue<QueryFilterRef> & GetChildren() {return _children;}

private:
   Queue<QueryFilterRef> _children;
};

/** This class matches iff at least (n) of its children match.  As such, it can be used as an OR operator,
  * an AND operator, or something in-between the two.
  */
class AndOrQueryFilter : public MultiQueryFilter
{
public:
   /** Default constructor.  Creates an AND filter with no children. 
     * @param minMatches The minimum number of children that must match before this filter considers
     *                   the match to be valid.  Default to ((uint32)-1), meaning all children must match.
     */
   AndOrQueryFilter(uint32 minMatches = ((uint32)-1)) : _minMatches(minMatches) {/* empty */}

   /** Convenience constructor for simple binary 'or' or 'and' operations.
     * @param child1 First argument to the operation
     * @param child2 Second argument to the operation
     * @param isAnd If true, the operation will be an 'and' operation.  Otherwise it will be an 'or' operation.
     */
   AndOrQueryFilter(const QueryFilterRef & child1, const QueryFilterRef & child2, bool isAnd) : _minMatches(isAnd ? ((uint32)-1) : 1)
   {
      GetChildren().AddTail(child1);
      GetChildren().AddTail(child2);
   }

   virtual status_t SaveToArchive(Message & archive) const;
   virtual status_t SetFromArchive(const Message & archive);
   virtual uint32 TypeCode() const {return QUERY_FILTER_TYPE_ANDOR;}
   virtual bool Matches(const Message & msg, const DataNode * optNode) const;

   /** Set the minimum number of children that must match the target Message in order for this
     * filter to match the target Message.  If the specified number is greater than the number of
     * child filters held by this QueryFilter, then this filter matches only if every one of the child
     * filters match.
     * @param minMatches How many child filters must match, at a minimum.
     */
   void SetMinMatchCount(uint32 minMatches) {_minMatches = minMatches;}

   /** Returns the minimum-match-count for this filter, as specified in the constructor or by SetMinMatchCount(). */
   uint32 GetMinMatchCount() const {return _minMatches;}

private:
   uint32 _minMatches;
};

/** This class matches iff at most (n) of its children match.  As such, it can be used as a NAND operator,
  * a NOT operator, or something in-between the two.
  */
class NandNotQueryFilter : public MultiQueryFilter
{
public:
   /** Default constructor.  Creates an NAND filter with no children. 
     * @param maxMatches The maximum number of children that may match before this filter considers
     *                   the match to be invalid.  Defaults to 0, meaning no children may match.
     */
   NandNotQueryFilter(uint32 maxMatches = 0) : _maxMatches(maxMatches) {/* empty */}

   /** Convenience constructor for simple unary 'not' operation.
     * @param child Child whose logic we should negate.  This child is added to our child list, and the MaxMatchCount is set to zero. 
     */
   NandNotQueryFilter(const QueryFilterRef & child) : _maxMatches(0)
   {
      GetChildren().AddTail(child);
   }

   /** Convenience constructor for simple binary 'nand' operation.  MaxMatchCount is set to one.
     * @param child1 First argument to the operation
     * @param child2 Second argument to the operation
     */
   NandNotQueryFilter(const QueryFilterRef & child1, const QueryFilterRef & child2) : _maxMatches(1)
   {
      GetChildren().AddTail(child1);
      GetChildren().AddTail(child2);
   }

   virtual status_t SaveToArchive(Message & archive) const;
   virtual status_t SetFromArchive(const Message & archive);
   virtual uint32 TypeCode() const {return QUERY_FILTER_TYPE_NANDNOT;}
   virtual bool Matches(const Message & msg, const DataNode * optNode) const;

   /** Set the maximum number of children that may match the target Message in order for this
     * filter to match the target Message.  If the specified number is greater than the number of
     * child filters held by this QueryFilter, then this filter fails to match only if every one 
     * of the child filters match.
     * @param maxMatches Maximum number of child filters that may match.
     */
   void SetMaxMatchCount(uint32 maxMatches) {_maxMatches = maxMatches;}

   /** Returns the maximum-match-count for this filter, as specified in the constructor or by SetMaxMatchCount(). */
   uint32 GetMaxMatchCount() const {return _maxMatches;}

private:
   uint32 _maxMatches;
};

/** This class matches only if an odd number of its children match. */
class XorQueryFilter : public MultiQueryFilter
{
public:
   /** Default constructor.  You'll want to add children to this object manually. */
   XorQueryFilter() {/* empty */}

   /** Convenience constructor for simple binary 'xor' operation.
     * @param child1 First argument to the operation
     * @param child2 Second argument to the operation
     */
   XorQueryFilter(const QueryFilterRef & child1, const QueryFilterRef & child2)
   {
      GetChildren().AddTail(child1);
      GetChildren().AddTail(child2);
   }

   virtual uint32 TypeCode() const {return QUERY_FILTER_TYPE_XOR;}
   virtual bool Matches(const Message & msg, const DataNode * optNode) const;
};

/** This class matches iff the specified sub-Message exists in our target Message,
  * and (optionally) our child QueryFilterRef can match that sub-Message.
  */
class MessageQueryFilter : public ValueQueryFilter
{
public:
   /** Default constructor.  The child filter is left NULL, so that any child Message will match */
   MessageQueryFilter() {/* Empty */}

   /** Constructor.
     * @param childFilter Reference to the filter to use to match the sub-Message found at (fieldName:index)
     * @param fieldName Name of the field in the Message to look at
     * @param index Index of the item within the field to look at.  Defaults to zero (i.e. the first item)
     */
   MessageQueryFilter(const QueryFilterRef & childFilter, const String & fieldName, uint32 index = 0) : ValueQueryFilter(fieldName, index), _childFilter(childFilter) {/* empty */}

   virtual status_t SaveToArchive(Message & archive) const;
   virtual status_t SetFromArchive(const Message & archive);
   virtual uint32 TypeCode() const {return QUERY_FILTER_TYPE_MESSAGE;}
   virtual bool Matches(const Message & msg, const DataNode * optNode) const;

   /** Set the sub-filter to use on the target's sub-Message.
     * @param childFilter Filter to use, or a NULL reference to indicate that any sub-Message found should match. 
     */
   void SetChildFilter(const QueryFilterRef & childFilter) {_childFilter = childFilter;}

   /** Returns our current sub-filter as set in our constructor or in SetChildFilter() */
   QueryFilterRef GetChildFilter() const {return _childFilter;}

private:
   QueryFilterRef _childFilter;
};

/** This class matches on string field values.  */
class StringQueryFilter : public ValueQueryFilter
{
public:
   /** Default constructor.  The string is set to "", and the operator is set to OP_EQUAL_TO. */
   StringQueryFilter() : _op(OP_EQUAL_TO), _matcher(NULL) {/* Empty */}

   /** Constructor.
     * @param fieldName Field name to look under.
     * @param op The operator to use (should be one of the OP_* values enumerated below)
     * @param value The string to compare to the string found in the Message.
     * @param index Optional index of the item within the field.  Defaults to zero.
     */
   StringQueryFilter(const String & fieldName, uint8 op, const String & value, uint32 index = 0) : ValueQueryFilter(fieldName, index), _value(value), _op(op), _matcher(NULL) {/* empty */}

   /** Destructor */
   ~StringQueryFilter() {FreeMatcher();}

   virtual status_t SaveToArchive(Message & archive) const;
   virtual status_t SetFromArchive(const Message & archive);
   virtual uint32 TypeCode() const {return QUERY_FILTER_TYPE_STRING;}
   virtual bool Matches(const Message & msg, const DataNode * optNode) const;

   /** Set the operator to use.  
     * @param op One of the OP_* values enumerated below.
     */
   void SetOperator(uint8 op) {if (op != _op) {_op = op; FreeMatcher();}}

   /** Returns the currently specified operator, as specified in the constructor or in SetOperator() */
   uint8 GetOperator() const {return _op;}

   /** Set the value to compare against.
     * @param value The new value.
     */
   void SetValue(const String & value) {if (value != _value) {_value = value; FreeMatcher();}}

   /** Returns the currently specified value, as specified in the constructor or in SetValue() */
   const String & GetValue() const {return _value;}
 
   enum {
      OP_EQUAL_TO = 0,
      OP_LESS_THAN,
      OP_GREATER_THAN,
      OP_LESS_THAN_OR_EQUAL_TO,
      OP_GREATER_THAN_OR_EQUAL_TO,
      OP_NOT_EQUAL_TO,
      OP_STARTS_WITH,
      OP_ENDS_WITH,
      OP_CONTAINS,
      OP_START_OF,
      OP_END_OF,
      OP_SUBSTRING_OF,
      OP_EQUAL_TO_IGNORECASE,
      OP_LESS_THAN_IGNORECASE,
      OP_GREATER_THAN_IGNORECASE,
      OP_LESS_THAN_OR_EQUAL_TO_IGNORECASE,
      OP_GREATER_THAN_OR_EQUAL_TO_IGNORECASE,
      OP_NOT_EQUAL_TO_IGNORECASE,
      OP_STARTS_WITH_IGNORECASE,
      OP_ENDS_WITH_IGNORECASE,
      OP_CONTAINS_IGNORECASE,
      OP_START_OF_IGNORECASE,
      OP_END_OF_IGNORECASE,
      OP_SUBSTRING_OF_IGNORECASE,
      OP_SIMPLE_WILDCARD_MATCH,
      OP_REGULAR_EXPRESSION_MATCH,
      NUM_STRING_OPERATORS
   };

private:
   void FreeMatcher();
   bool DoMatch(const String & s) const;

   String _value;
   uint8 _op;
   mutable StringMatcher * _matcher;
};

/** This class matches on raw data buffers.  */
class RawDataQueryFilter : public ValueQueryFilter
{
public:
   /** Default constructor.  The byte buffer is set to a zero-length/NULL buffer, and the operator is set to OP_EQUAL_TO. */
   RawDataQueryFilter() : _op(OP_EQUAL_TO) {/* Empty */}

   /** Constructor.
     * @param fieldName Field name to look under.
     * @param op The operator to use (should be one of the OP_* values enumerated below)
     * @param value The string to compare to the string found in the Message.
     * @param typeCode Typecode to look for in the target Message.  Default is B_ANY_TYPE, indicating that any type code is acceptable.
     * @param index Optional index of the item within the field.  Defaults to zero.
     */
   RawDataQueryFilter(const String & fieldName, uint8 op, const ByteBufferRef & value, uint32 typeCode = B_ANY_TYPE, uint32 index = 0) : ValueQueryFilter(fieldName, index), _value(value), _op(op), _typeCode(typeCode) {/* empty */}

   virtual status_t SaveToArchive(Message & archive) const;
   virtual status_t SetFromArchive(const Message & archive);
   virtual uint32 TypeCode() const {return QUERY_FILTER_TYPE_RAWDATA;}
   virtual bool Matches(const Message & msg, const DataNode * optNode) const;

   /** Set the operator to use.  
     * @param op One of the OP_* values enumerated below.
     */
   void SetOperator(uint8 op) {_op = op;}

   /** Returns the currently specified operator, as specified in the constructor or in SetOperator() */
   uint8 GetOperator() const {return _op;}

   /** Set the value to compare against.
     * @param value The new value.
     */
   void SetValue(const ByteBufferRef & value) {_value = value;}

   /** Sets the type code that we will look for in the target Message.
     * @param typeCode the type code to look for.  Use B_ANY_TYPE to indicate that you don't care what the type code is.
     */
   void SetTypeCode(uint32 typeCode) {_typeCode = typeCode;}

   /** Returns the type code we are to look in the target Message for.
     * Note that this method is different from TypeCode()!
     */
   uint32 GetTypeCode() const {return _typeCode;}

   /** Returns the currently specified value, as specified in the constructor or in SetValue() */
   ByteBufferRef GetValue() const {return _value;}
 
   enum {
      OP_EQUAL_TO = 0,
      OP_LESS_THAN,
      OP_GREATER_THAN,
      OP_LESS_THAN_OR_EQUAL_TO,
      OP_GREATER_THAN_OR_EQUAL_TO,
      OP_NOT_EQUAL_TO,
      OP_STARTS_WITH,
      OP_ENDS_WITH,
      OP_CONTAINS,
      OP_START_OF,
      OP_END_OF,
      OP_SUBSET_OF,
      NUM_RAWDATA_OPERATORS
   };

private:
   const uint8 * Memmem(const uint8 * lookIn, uint32 numLookInBytes, const uint8 * lookFor, uint32 numLookForBytes) const;

   ByteBufferRef _value;
   uint8 _op;
   uint32 _typeCode;
};

/** Interface for any object that knows how to instantiate QueryFilter objects */
class QueryFilterFactory : public RefCountable
{
public:
   QueryFilterFactory() {/* empty */}
   virtual ~QueryFilterFactory() {/* empty */}

   /** Attempts to create and return a QueryFilter object from the given typeCode.
     * @param typeCode One of the QUERY_FILTER_TYPE_* values enumerated above.
     * @returns Reference to the new QueryFilter object on success, or a NULL reference on failure.
     */
   virtual QueryFilterRef CreateQueryFilter(uint32 typeCode) const = 0;

   /** Convenience method:  Attempts to create, populate, and return a QueryFilter object from 
     *                      the given Message, by first calling CreateQueryFilter(msg.what),
     *                      and then calling SetFromArchive(msg) on the return QueryFilter object.
     * @param msg A Message object that was previously filled out by the SaveToArchive() method
     *            of a QueryFilter object.
     * @returns Reference to the new QueryFilter object on success, or a NULL reference on failure.
     * @note Do not override this method in subclasses; override CreateQueryFilter(uint32) instead.
     */
   QueryFilterRef CreateQueryFilter(const Message & msg) const;
};

/** This class is MUSCLE's built-in implementation of a QueryFilterFactory.
  * It knows how to create all of the filter types listed in the QUERY_FILTER_TYPE_*
  * enum above.
  */
class MuscleQueryFilterFactory : public QueryFilterFactory
{
public:
   MuscleQueryFilterFactory() {/* empty */}

   virtual QueryFilterRef CreateQueryFilter(uint32 typeCode) const;
};

/** Type for a reference to a queue of StringMatcher objects. */
typedef Ref<QueryFilterFactory> QueryFilterFactoryRef;

/** Returns a reference to the globally installed QueryFilterFactory object
  * that is used to create QueryFilter objects.  This method is guaranteed
  * never to return a NULL reference -- even if you call 
  * SetglobalQueryFilterFactory(QueryFilterFactoryRef()), this method
  * will fall back to returning a reference to a MuscleQueryFilterFactory
  * object (which is also what it does by default).
  */
QueryFilterFactoryRef GetGlobalQueryFilterFactory();

/** Call this method if you want to install a custom QueryFilterFactory
  * object as the global QueryFilterFactory.  Calling this method with
  * a NULL reference will revert the system back to using the default
  * MuscleQueryFilterFactory object.
  */
void SetGlobalQueryFilterFactory(const QueryFilterFactoryRef & newFactory);

END_NAMESPACE(muscle);

#endif
