#!/bin/sh

# 현재 디렉토리에 있는 파일들과 비교할 디렉토리를 변수로 저장합니다.
current_directory="./log_file"
other_directory="../test_log/log_file"

# 현재 디렉토리에 있는 파일 목록을 배열에 저장합니다.
files=$(ls -1 $current_directory)

# 현재 디렉토리의 각 파일과 다른 경로에 있는 동일한 파일 이름을 비교합니다.
for file in $files; do
  echo "Comparing $file..."
  diff "${current_directory}/${file}" "${other_directory}/${file}"
done
