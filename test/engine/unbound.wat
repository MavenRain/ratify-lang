;; enginetool refuses this module at load: no import of the table binding
;; (test/enginetool.c) is ratify.nope. The module has no entry to call.
(module
  (import "ratify" "nope" (func (param i64) (result i64)))
  (func (result i64) i64.const 0))
