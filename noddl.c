// Standard set of includes for an extension.

#include "postgres.h"
#include "utils/builtins.h"
#include "utils/elog.h"
#include "utils/guc.h"

// These includes are required for all of the query and plan parsing parameters
// to the ProcessUtility hook function.

#include "optimizer/planner.h" // For PlannedStmt, ParamListInfo
#include "parser/parse_node.h" // For QueryEnvironment
#include "tcop/utility.h" // For ProcessUtilityContext

#include "noddl.h"

bool	deny_ddl = false;

ProcessUtility_hook_type next_ProcessUtility_hook = NULL;

PG_MODULE_MAGIC;

static void
noddl_ProcessUtility(
    PlannedStmt *pstmt,
    const char *queryString,
    bool readOnlyTree,
    ProcessUtilityContext context,
    ParamListInfo params,
    QueryEnvironment *queryEnv,
    DestReceiver *dest,
    QueryCompletion *qc)
{
  Node *parsetree = pstmt->utilityStmt;

  /* The core of this function is extremely simple:
   * 
   * If our GUC is enabled, and this is a DDL statement, immediately abort.
   * Maybe this could be converted to a switch statement to deny specific DDL
   * operations instead, but for this PoC, we just deny freaking everything.
   */

  if (deny_ddl && GetCommandLogLevel(parsetree) == LOGSTMT_DDL)
      ereport(ERROR,
        (errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
          errmsg("DDL is not allowed on this system."),
          errhint("Disable the noddl.enable GUC to continue."))
      );

  /* Any time we don't abort, we need to explicitly call the next registered
   * ProcessUtility afterwards, or not even allowed SQL statements would be
   * processed. While highly amusing, that's not the intent of this extension.
   */

  if (next_ProcessUtility_hook)
    next_ProcessUtility_hook(
        pstmt, queryString, readOnlyTree, context, params,
        queryEnv, dest, qc
    );
  else
    standard_ProcessUtility(
        pstmt, queryString, readOnlyTree, context, params,
        queryEnv, dest, qc
    );

} // noddl_ProcessUtility


void
_PG_init(void)
{
  DefineCustomBoolVariable(
    "noddl.enable",
    "Deny All DDL statements",
    NULL,
    &deny_ddl,
    false,
    PGC_SUSET,          // Only allow superusers to modify
    0,                  // No flags for this GUC
    NULL, NULL, NULL    // No hooks necessary
  );

  if (ProcessUtility_hook)
    next_ProcessUtility_hook = ProcessUtility_hook;

  ProcessUtility_hook = noddl_ProcessUtility;

} // _PG_init
