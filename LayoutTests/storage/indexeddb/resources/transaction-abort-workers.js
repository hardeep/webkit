if (this.importScripts) {
    importScripts('../../../fast/js/resources/js-test-pre.js');
    importScripts('shared.js');
}

description("Test IndexedDB workers, recursion, and transaction termination.");

function test()
{
    shouldBeTrue("'webkitIndexedDB' in self");
    shouldBeFalse("webkitIndexedDB == null");

    shouldBeTrue("'webkitIDBCursor' in self");
    shouldBeFalse("webkitIDBCursor == null");

    evalAndLog("request = webkitIndexedDB.open('transaction-abort-worker')");
    request.onerror = unexpectedErrorCallback;
    request.onsuccess = function () {
        evalAndLog("db = request.result");
        evalAndLog("request = db.setVersion('1')");
        request.onerror = unexpectedErrorCallback;
        request.onblocked = unexpectedBlockedCallback;
        request.onsuccess = function () {
            deleteAllObjectStores(db);
            evalAndLog("trans = request.result");
            evalAndLog("db.createObjectStore('store')");
            trans.oncomplete = createTransaction;
        };
    };
}

function createTransaction()
{
    debug("");
    debug("createTransaction():");
    evalAndLog("transaction = db.transaction('store')");
    evalAndLog("store = transaction.objectStore('store')");
    transaction.oncomplete = unexpectedCompleteCallback;
    transaction.onerror = unexpectedErrorCallback;
    transaction.onabort = transactionAborted;
}

function transactionAborted()
{
    testPassed("Transaction aborted");
    evalAndExpectException("store.get(0)", "webkitIDBDatabaseException.TRANSACTION_INACTIVE_ERR");
    recursionTest();
}

function recursionTest()
{
    debug("");
    debug("recursionTest():");
    evalAndLog("transaction = db.transaction('store')");
    evalAndLog("store = transaction.objectStore('store')");
    transaction.oncomplete = transactionCompleted;
    transaction.onabort = unexpectedAbortCallback;
    evalAndLog("store.get(0)");
    testPassed("transaction is active");
    recurse(1);
}

function recurse(count)
{
    debug("recursion depth: " + count);
    evalAndLog("store.get(0)");
    testPassed("transaction is still active");
    if (count < 3) {
        recurse(count + 1);
    }
    debug("recursion depth: " + count);
    evalAndLog("store.get(0)");
    testPassed("transaction is still active");
}

function transactionCompleted()
{
    testPassed("transaction completed");
    evalAndExpectException("store.get(0)", "webkitIDBDatabaseException.TRANSACTION_INACTIVE_ERR");

    debug("");
    debug("trying a timeout callback:");
    evalAndLog("setTimeout(timeoutTest, 0)");
}

function timeoutTest()
{
    debug("");
    debug("timeoutTest():");

    evalAndLog("transaction = db.transaction('store')");
    evalAndLog("store = transaction.objectStore('store')");
    transaction.oncomplete = unexpectedCompleteCallback;
    transaction.onabort = function () {
        testPassed("transaction started in setTimeout() callback aborted");
        evalAndExpectException("store.get(0)", "webkitIDBDatabaseException.TRANSACTION_INACTIVE_ERR");

        errorTest();
    };
}

function errorTest()
{
    debug("");
    debug("errorTest():");
    evalAndLog("self.old_onerror = self.onerror");
    evalAndLog("self.onerror = errorHandler");
    throw new Error("ignore this");
}

function errorHandler(e)
{
    debug("");
    debug("errorHandler():");
    // FIXME: Should be able to stop the error here, but it isn't an Event object.
    // evalAndLog("event.preventDefault()");
    evalAndLog("self.onerror = self.old_onerror");
    evalAndLog("transaction = db.transaction('store')");
    evalAndLog("store = transaction.objectStore('store')");
    transaction.oncomplete = unexpectedCompleteCallback;
    transaction.onerror = unexpectedErrorCallback;
    transaction.onabort = errorTransactionAborted;
}

function errorTransactionAborted()
{
    testPassed("Transaction aborted");
    evalAndExpectException("store.get(0)", "webkitIDBDatabaseException.TRANSACTION_INACTIVE_ERR");
    finishJSTest();
}

test();
