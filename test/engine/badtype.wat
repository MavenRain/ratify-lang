;; enginetool refuses this module at load: the table import ratify.hash_eq
;; has 4 parameters and this import has 2.
(module
  (import "ratify" "hash_eq" (func (param i64 i64) (result i64)))
  (func (result i64) i64.const 0))
