/*-------------------------------------------------------------------------
 *
 * rangetypes.h
 *	  Declarations for Postgres range types.
 *
 *
 * Portions Copyright (c) 1996-2011, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/utils/rangetypes.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef RANGETYPES_H
#define RANGETYPES_H

#include "utils/typcache.h"


/*
 * Ranges are varlena objects, so must meet the varlena convention that
 * the first int32 of the object contains the total object size in bytes.
 * Be sure to use VARSIZE() and SET_VARSIZE() to access it, though!
 */
typedef struct
{
	int32		vl_len_;		/* varlena header (do not touch directly!) */
	Oid			rangetypid;		/* range type's own OID */
	/* Following the OID are zero to two bound values, then a flags byte */
} RangeType;

/* Use this macro in preference to fetching rangetypid field directly */
#define RangeTypeGetOid(r)  ((r)->rangetypid)

/* A range's flags byte contains these bits: */
#define RANGE_EMPTY		0x01	/* range is empty */
#define RANGE_LB_INC	0x02	/* lower bound is inclusive (vs exclusive) */
#define RANGE_LB_NULL	0x04	/* lower bound is null (NOT CURRENTLY USED) */
#define RANGE_LB_INF	0x08	/* lower bound is +/- infinity */
#define RANGE_UB_INC	0x10	/* upper bound is inclusive (vs exclusive) */
#define RANGE_UB_NULL	0x20	/* upper bound is null (NOT CURRENTLY USED) */
#define RANGE_UB_INF	0x40	/* upper bound is +/- infinity */

#define RANGE_HAS_LBOUND(flags) (!((flags) & (RANGE_EMPTY | \
											  RANGE_LB_NULL | \
											  RANGE_LB_INF)))

#define RANGE_HAS_UBOUND(flags) (!((flags) & (RANGE_EMPTY | \
											  RANGE_UB_NULL | \
											  RANGE_UB_INF)))


/* Internal representation of either bound of a range (not what's on disk) */
typedef struct
{
	Datum		val;			/* the bound value, if any */
	bool		infinite;		/* bound is +/- infinity */
	bool		inclusive;		/* bound is inclusive (vs exclusive) */
	bool		lower;			/* this is the lower (vs upper) bound */
} RangeBound;

/*
 * fmgr macros for range type objects
 */
#define DatumGetRangeType(X)		((RangeType *) PG_DETOAST_DATUM(X))
#define DatumGetRangeTypeCopy(X)	((RangeType *) PG_DETOAST_DATUM_COPY(X))
#define RangeTypeGetDatum(X)		PointerGetDatum(X)
#define PG_GETARG_RANGE(n)			DatumGetRangeType(PG_GETARG_DATUM(n))
#define PG_GETARG_RANGE_COPY(n)		DatumGetRangeTypeCopy(PG_GETARG_DATUM(n))
#define PG_RETURN_RANGE(x)			return RangeTypeGetDatum(x)

/*
 * prototypes for functions defined in rangetypes.c
 */

/* I/O */
extern Datum range_in(PG_FUNCTION_ARGS);
extern Datum range_out(PG_FUNCTION_ARGS);
extern Datum range_recv(PG_FUNCTION_ARGS);
extern Datum range_send(PG_FUNCTION_ARGS);

/* constructors */
extern Datum range_constructor0(PG_FUNCTION_ARGS);
extern Datum range_constructor1(PG_FUNCTION_ARGS);
extern Datum range_constructor2(PG_FUNCTION_ARGS);
extern Datum range_constructor3(PG_FUNCTION_ARGS);

/* range -> subtype */
extern Datum range_lower(PG_FUNCTION_ARGS);
extern Datum range_upper(PG_FUNCTION_ARGS);

/* range -> bool */
extern Datum range_empty(PG_FUNCTION_ARGS);
extern Datum range_lower_inc(PG_FUNCTION_ARGS);
extern Datum range_upper_inc(PG_FUNCTION_ARGS);
extern Datum range_lower_inf(PG_FUNCTION_ARGS);
extern Datum range_upper_inf(PG_FUNCTION_ARGS);

/* range, element -> bool */
extern Datum range_contains_elem(PG_FUNCTION_ARGS);
extern Datum elem_contained_by_range(PG_FUNCTION_ARGS);

/* range, range -> bool */
extern Datum range_eq(PG_FUNCTION_ARGS);
extern Datum range_ne(PG_FUNCTION_ARGS);
extern Datum range_contains(PG_FUNCTION_ARGS);
extern Datum range_contained_by(PG_FUNCTION_ARGS);
extern Datum range_before(PG_FUNCTION_ARGS);
extern Datum range_after(PG_FUNCTION_ARGS);
extern Datum range_adjacent(PG_FUNCTION_ARGS);
extern Datum range_overlaps(PG_FUNCTION_ARGS);
extern Datum range_overleft(PG_FUNCTION_ARGS);
extern Datum range_overright(PG_FUNCTION_ARGS);

/* range, range -> range */
extern Datum range_minus(PG_FUNCTION_ARGS);
extern Datum range_union(PG_FUNCTION_ARGS);
extern Datum range_intersect(PG_FUNCTION_ARGS);

/* BTree support */
extern Datum range_cmp(PG_FUNCTION_ARGS);
extern Datum range_lt(PG_FUNCTION_ARGS);
extern Datum range_le(PG_FUNCTION_ARGS);
extern Datum range_ge(PG_FUNCTION_ARGS);
extern Datum range_gt(PG_FUNCTION_ARGS);

/* Hash support */
extern Datum hash_range(PG_FUNCTION_ARGS);

/* Canonical functions */
extern Datum int4range_canonical(PG_FUNCTION_ARGS);
extern Datum int8range_canonical(PG_FUNCTION_ARGS);
extern Datum daterange_canonical(PG_FUNCTION_ARGS);

/* Subtype Difference functions */
extern Datum int4range_subdiff(PG_FUNCTION_ARGS);
extern Datum int8range_subdiff(PG_FUNCTION_ARGS);
extern Datum numrange_subdiff(PG_FUNCTION_ARGS);
extern Datum daterange_subdiff(PG_FUNCTION_ARGS);
extern Datum tsrange_subdiff(PG_FUNCTION_ARGS);
extern Datum tstzrange_subdiff(PG_FUNCTION_ARGS);

/* assorted support functions */
extern TypeCacheEntry *range_get_typcache(FunctionCallInfo fcinfo,
										  Oid rngtypid);
extern RangeType *range_serialize(TypeCacheEntry *typcache, RangeBound *lower,
							 RangeBound *upper, bool empty);
extern void range_deserialize(TypeCacheEntry *typcache, RangeType *range,
							  RangeBound *lower, RangeBound *upper,
							  bool *empty);
extern char range_get_flags(RangeType *range);
extern RangeType *make_range(TypeCacheEntry *typcache, RangeBound *lower,
						RangeBound *upper, bool empty);
extern int range_cmp_bounds(TypeCacheEntry *typcache, RangeBound *b1,
							RangeBound *b2);
extern RangeType *make_empty_range(TypeCacheEntry *typcache);

/* GiST support (in rangetypes_gist.c) */
extern Datum range_gist_consistent(PG_FUNCTION_ARGS);
extern Datum range_gist_compress(PG_FUNCTION_ARGS);
extern Datum range_gist_decompress(PG_FUNCTION_ARGS);
extern Datum range_gist_union(PG_FUNCTION_ARGS);
extern Datum range_gist_penalty(PG_FUNCTION_ARGS);
extern Datum range_gist_picksplit(PG_FUNCTION_ARGS);
extern Datum range_gist_same(PG_FUNCTION_ARGS);

#endif   /* RANGETYPES_H */
