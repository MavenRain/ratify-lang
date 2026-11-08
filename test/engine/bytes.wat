;; enginetool --bytes reads the exported memory of this module with
;; engine_bytes (S10 U1): one page (65536 bytes), with the 8 bytes 01 to 08
;; at 65528, the last word.
(module
  (memory (export "memory") 1 1)
  (data (i32.const 65528) "\01\02\03\04\05\06\07\08"))
