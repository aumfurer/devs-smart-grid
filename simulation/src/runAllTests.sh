#!/bin/bash

TESTS_DIRECTORY=../tests
SIMULATOR=bin/cd++
TEST_OUTPUT=output

# TODO: Compile again whole simulator, to use it a test and a hook

runTest() {
    # TODO: Save test output to a file?
    $SIMULATOR -m$TESTS_DIRECTORY/$1.ma -e$TESTS_DIRECTORY/$1.ev -o$TEST_OUTPUT > /dev/null 2>&1
}

testsToRun=($(ls $TESTS_DIRECTORY | sed -E 's/([a-zA-Z]*)\..*/\1/g' | uniq))

for test in "${testsToRun[@]}"
do
    echo -n "Running test: $test..."
    runTest $test
    diff $TEST_OUTPUT $TESTS_DIRECTORY/$test.expected

    if [ $? -eq 0 ]
    then
        # Test passed
        echo "    PASSED"
    else
        # Test failed
        echo "    FAILED"
    fi
done

# TODO: Give a results summary