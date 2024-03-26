
//#ifdef FATAL
//#undef FATAL
//#endif

#include "duckdb/main/client_context.hpp"
#include "duckdb/function/replacement_scan.hpp"
#include "duckdb/parser/tableref/table_function_ref.hpp"
#include "duckdb/parser/expression/function_expression.hpp"
#include "duckdb/parser/expression/constant_expression.hpp"

#include "quack.hpp"
#include "quack_scan.hpp"

extern "C" {

#include "postgres.h"

#include "miscadmin.h"

#include "access/tableam.h"
#include "executor/executor.h"
#include "parser/parse_type.h"
#include "tcop/utility.h"
#include "catalog/pg_type.h"
#include "utils/syscache.h"
#include "utils/builtins.h"
}

namespace duckdb {

// ------- Table Function -------

PostgresScanFunction::PostgresScanFunction()
    : TableFunction("postgres_scan", {LogicalType::POINTER}, PostgresFunc, PostgresBind, PostgresInitGlobal,
                    PostgresInitLocal) {
}

// Bind Data

PostgresScanFunctionData::PostgresScanFunctionData(RangeTblEntry *table) : table(table) {
}

PostgresScanFunctionData::~PostgresScanFunctionData() {
}

static LogicalType PostgresToDuck(Oid type) {
	switch (type) {
	case BOOLOID:
		return LogicalTypeId::BOOLEAN;
	case CHAROID:
		return LogicalTypeId::TINYINT;
	case INT2OID:
		return LogicalTypeId::SMALLINT;
	case INT4OID:
		return LogicalTypeId::INTEGER;
	case INT8OID:
		return LogicalTypeId::BIGINT;
	case BPCHAROID:
	case TEXTOID:
	case VARCHAROID:
		return LogicalTypeId::VARCHAR;
	case DATEOID:
		return LogicalTypeId::DATE;
	case TIMESTAMPOID:
		return LogicalTypeId::TIMESTAMP;
	default:
		elog(ERROR, "Unsupported quack type: %d", type);
	}
}

unique_ptr<FunctionData> PostgresScanFunction::PostgresBind(ClientContext &context, TableFunctionBindInput &input,
                                                            vector<LogicalType> &return_types, vector<string> &names) {
	auto table = (reinterpret_cast<RangeTblEntry *>(input.inputs[0].GetPointer()));

	D_ASSERT(table->relid);
	auto rel = RelationIdGetRelation(table->relid);

	auto tupleDesc = RelationGetDescr(rel);
	if (!tupleDesc) {
		elog(ERROR, "Failed to get tuple descriptor for relation with OID %u", table->relid);
		RelationClose(rel);
		return nullptr;
	}

	int column_count = tupleDesc->natts;

	for (idx_t i = 0; i < column_count; i++) {
		Form_pg_attribute attr = &tupleDesc->attrs[i];
		Oid type_oid = attr->atttypid;
		auto col_name = string(NameStr(attr->attname));
		auto duck_type = PostgresToDuck(type_oid);
		return_types.push_back(duck_type);
		names.push_back(col_name);

		/* Log column name and type */
		elog(INFO, "Column name: %s, Type: %s", col_name.c_str(), duck_type.ToString().c_str());
	}

	// FIXME: check this in the replacement scan
	D_ASSERT(rel->rd_amhandler != 0);
	// These are the methods we need to interact with the table
	auto access_method_handler = GetTableAmRoutine(rel->rd_amhandler);
	RelationClose(rel);

	return make_uniq<PostgresScanFunctionData>(table);
}

// Global State

PostgresScanGlobalState::PostgresScanGlobalState() {
}

unique_ptr<GlobalTableFunctionState> PostgresScanFunction::PostgresInitGlobal(ClientContext &context,
                                                                              TableFunctionInitInput &input) {
	auto bind_data = input.bind_data->Cast<PostgresScanFunctionData>();
	auto table = bind_data.table;
	return make_uniq<PostgresScanGlobalState>();
}

// Local State

PostgresScanLocalState::PostgresScanLocalState() {
}

unique_ptr<LocalTableFunctionState> PostgresScanFunction::PostgresInitLocal(ExecutionContext &context,
                                                                            TableFunctionInitInput &input,
                                                                            GlobalTableFunctionState *gstate) {
	return make_uniq<PostgresScanLocalState>();
}

// The table scan function

void PostgresScanFunction::PostgresFunc(ClientContext &context, TableFunctionInput &data_p, DataChunk &output) {
	return;
}

// ------- Replacement Scan -------

PostgresReplacementScanData::PostgresReplacementScanData(QueryDesc *desc) : desc(desc) {
}
PostgresReplacementScanData::~PostgresReplacementScanData() {
}

static RangeTblEntry *FindMatchingRelation(List *tables, const string &to_find) {
	ListCell *lc;
	foreach (lc, tables) {
		RangeTblEntry *table = (RangeTblEntry *)lfirst(lc);
		if (table->relid) {
			auto rel = RelationIdGetRelation(table->relid);

			if (!RelationIsValid(rel)) {
				elog(ERROR, "Relation with OID %u is not valid", table->relid);
				return nullptr;
			}

			char *relName = RelationGetRelationName(rel);
			auto table_name = std::string(relName);
			if (StringUtil::CIEquals(table_name, to_find)) {
				if (!rel->rd_amhandler) {
					// This doesn't have an access method handler, we cant read from this
					RelationClose(rel);
					return nullptr;
				}
				RelationClose(rel);
				return table;
			}
			RelationClose(rel);
		}
	}
	return nullptr;
}

unique_ptr<TableRef> PostgresReplacementScan(ClientContext &context, const string &table_name,
                                             ReplacementScanData *data) {
	auto &scan_data = reinterpret_cast<PostgresReplacementScanData &>(*data);
	// Use 'QueryDesc *desc' to query the postgres table
	// We will return a custom table function scan with parameters (likely passing a pointer as parameter)

	auto tables = scan_data.desc->plannedstmt->rtable;
	auto table = FindMatchingRelation(tables, table_name);
	if (!table) {
		elog(ERROR, "Failed to find table %s in replacement scan lookup", table_name.c_str());
		return nullptr;
	}

	// Then inside the table function we can scan tuples from the postgres table and convert them into duckdb vectors.
	auto table_function = make_uniq<TableFunctionRef>();
	vector<unique_ptr<ParsedExpression>> children;
	children.push_back(make_uniq<ConstantExpression>(Value::POINTER(CastPointerToValue(table))));
	table_function->function = make_uniq<FunctionExpression>("postgres_scan", std::move(children));
	return std::move(table_function);
}

} // namespace duckdb
