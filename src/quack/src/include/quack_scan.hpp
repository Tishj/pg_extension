#pragma once

#include "duckdb.hpp"
#include "postgres.h"
#include "miscadmin.h"

extern "C" {
#include "executor/executor.h"
} // extern "C"

namespace duckdb {

struct PostgresReplacementScanData : public ReplacementScanData {
public:
	PostgresReplacementScanData(QueryDesc *desc);
	~PostgresReplacementScanData() override;
public:
	QueryDesc *desc;
};

unique_ptr<TableRef> PostgresReplacementScan(ClientContext &context, const string &table_name, ReplacementScanData *data);

} // namespace duckdb
