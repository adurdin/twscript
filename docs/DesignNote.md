Design Note
===========

These scripts uses the `Editor -> Design Note` feature in Dromed to store
their configuration, in common with the behaviour of scripts provided by
other authors like those found in NVScript, tnhScript, and PublicScripts.

For some scripts, the Design Note may be left empty, in which case the
script's default settings are used. However, this is rarely useful, and in
practice the Design Note will contain one or more configuration parameters.
Each parameter consists of three parts:

- a name, something like `TWTrapSetSpeedDest`
- an equals sign (=)
- a value to set for the parameter

For example:

    TWTrapSetSpeedDest='SomeTerrPt'

The value you can specify for any given parameter depends on its *type*.
The documentation for each parameter says what type of value it expects you
to give it, and it may require any of the following:

- `float`: the value should be a real number, that is a number that can
  include a fractional part, like `3.1415`. Negative numbers are specified
  using `-`, eg: `-2.54`. Note that, in some cases, negatives can
  produce unexpected or undesirable behaviours if the script doesn't expect
  you to use them.
- `integer`: a 'whole number', one without any decimal part, eg: `3`.
  Negative numbers can be specified using `-`, eg: `-42`.
- `boolean`: a true or false value. The following are considered to be 'true'
  values, any words that do not start as describe, or the number `0`, are
  considered to be false:
    - Any word starting 't', 'T', 'y' or 'Y'
    - Any non-zero integer value
- `string`: any text, no special meaning is attached to it. Note that, if
  the string needs to contain a semicolon (;) you must enclose the string
  in single or double quotes.
- `time`: an integer that represents a period of time. Without any modifier,
  the the value is interpreted as a number of milliseconds, if you append
  `s` to the number (eg: `10s`) the value is interpreted as a number of
  seconds. If you place `m` after the number, it is interpreted as a number
  of minutes.
- `object`: a Dromed object name, or object ID.

For the `integer`, `boolean`, and `time` types, you may also use a quest
variable in place of a literal value. To do this, prepend the quest variable
name with `$`. For example, this will use the value specified in the quest
variable `platform_speed`:

    TWTrapSetSpeed=$platform_speed

Values *may* be enclosed in quotes, either single quotes or double quotes,
but this is not required *unless* you are specifying a string containing a
semicolon, in which case you must quote the string.

If more than one parameter is specified, semicolons are used to separate
them, for example:

    TWTrapSetSpeed=5;TWTrapSetSpeedDest='*TerrPt'