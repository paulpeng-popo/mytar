#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

function cleanup {
    rm -rf testdata test_tar testdata.tar mytar.log tar.log
    make clean > /dev/null
}

# 當程式結束時執行 cleanup
trap cleanup EXIT

# 清除舊的測試資料
cleanup

# 編譯 mytar
make

# 建立測試資料
rsync -a --exclude=.git . testdata
echo

# 測試1：mytar 建立的 tar 檔案是否能被系統 tar 解壓縮且內容相同
mkdir -p test_tar
./mytar -c testdata.tar testdata > /dev/null
tar -xf testdata.tar -C test_tar

test1=$(diff -r testdata test_tar/testdata)
if [ -z "$test1" ]; then
    echo -e "Test 1: ${GREEN}Pass${NC}"
    rm -rf test_tar testdata.tar
else
    echo -e "Test 1: ${RED}Fail${NC}"
    exit 1
fi

# 測試2：mytar 是否能解壓縮系統 tar 建立的 tar 檔案
tar -cf testdata.tar testdata
mv testdata test_tar
./mytar -x testdata.tar > /dev/null

test2=$(diff -r test_tar testdata)
if [ -z "$test2" ]; then
    echo -e "Test 2: ${GREEN}Pass${NC}"
    rm -rf test_tar testdata
else
    echo -e "Test 2: ${RED}Fail${NC}"
    exit 1
fi

# 測試3：mytar list 的結果是否與系統 tar list 的結果相同
./mytar -tv testdata.tar > mytar.log
tar -tvf testdata.tar > tar.log

test3=$(diff -b mytar.log tar.log)
if [ -z "$test3" ]; then
    echo -e "Test 3: ${GREEN}Pass${NC}"
else
    echo -e "Test 3: ${RED}Fail${NC}"
    cat mytar.log
    cat tar.log
    exit 1
fi
