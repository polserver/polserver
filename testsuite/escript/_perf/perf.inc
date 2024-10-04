const PERF_ARRAY_SIZE := 5000000;
// const PERF_ITERATIONS := 10;
var arr := array;

function setup()
  arr[1] := 1;
  arr[PERF_ARRAY_SIZE] := 1;

  for i := 1 to PERF_ARRAY_SIZE
    arr[i] := i + 2007;
  endfor
endfunction
