// Generates the Unicode character tables needed by the domain parser from the
// Unicode Character Database, as documented in
// <https://www.unicode.org/reports/tr44>.
//
// Usage:
//
//   node scripts/generate-unicode-tables.mjs <version> <ucd> <header> <source>
//
// where <version> is the Unicode version the data was taken from, <ucd> is a
// directory holding the data files, and <header> and <source> are the files to
// write.

import fs from 'node:fs'
import path from 'node:path'

const [version, ucd, header, source] = process.argv.slice(2)

if (!version || !ucd || !header || !source) {
  console.error('Usage: node scripts/generate-unicode-tables.mjs <version> <ucd> <header> <source>')
  process.exit(1)
}

const MAX_CODE_POINT = 0x10ffff
const CODE_POINTS = MAX_CODE_POINT + 1

// The status values of the IDNA mapping table, as encoded in
// `url_idna_range_t`. Deviation is folded into valid as the domain parser only
// ever performs nontransitional processing, for which deviation code points are
// left unchanged and accepted by the validity criteria.
const IDNA_VALID = 0
const IDNA_IGNORED = 1
const IDNA_MAPPED = 2
const IDNA_DISALLOWED = 3

// The Bidi_Class values referenced by the Bidi rule of RFC 5893. All other
// values are lumped together as they are never allowed within a label of a Bidi
// domain name.
const BIDI_CLASSES = {
  Left_To_Right: 1,
  Right_To_Left: 2,
  Arabic_Letter: 3,
  Arabic_Number: 4,
  European_Number: 5,
  European_Separator: 6,
  Common_Separator: 7,
  European_Terminator: 8,
  Other_Neutral: 9,
  Boundary_Neutral: 10,
  Nonspacing_Mark: 11,

  L: 1,
  R: 2,
  AL: 3,
  AN: 4,
  EN: 5,
  ES: 6,
  CS: 7,
  ET: 8,
  ON: 9,
  BN: 10,
  NSM: 11
}

// The Joining_Type values referenced by the ContextJ rules of RFC 5892. As with
// the Bidi_Class values, the remaining values are lumped together.
const JOINING_TYPES = {
  Non_Joining: 0,
  Left_Joining: 1,
  Right_Joining: 2,
  Dual_Joining: 3,
  Transparent: 4,

  U: 0,
  L: 1,
  R: 2,
  D: 3,
  T: 4
}

function read(file) {
  return fs.readFileSync(path.join(ucd, file), 'utf8')
}

// Iterates the data lines of a data file, stripping comments and yielding the
// semicolon separated fields of each line.
function* lines(file) {
  for (const line of read(file).split('\n')) {
    const data = line.split('#')[0].trim()

    if (data === '') continue

    yield data.split(';').map((field) => field.trim())
  }
}

// Iterates the `@missing` annotations of a data file, which give the values of
// the code points that the file leaves unlisted.
function* missing(file) {
  for (const line of read(file).split('\n')) {
    const match = line.match(/^#\s*@missing:\s*([^;]+);\s*(.+?)\s*$/)

    if (match !== null) yield [match[1].trim(), match[2].trim()]
  }
}

// Parses a code point or an inclusive range of code points, such as `0041` or
// `0041..005A`.
function range(field) {
  const [start, end = start] = field.split('..')

  return [parseInt(start, 16), parseInt(end, 16)]
}

function codePoints(field) {
  if (field === '') return []

  return field.split(' ').map((code) => parseInt(code, 16))
}

/**
 * IdnaMappingTable.txt
 *
 * https://www.unicode.org/reports/tr46/#IDNA_Mapping_Table
 */

// Code points missing from the table are disallowed.
const idnaStatuses = new Uint8Array(CODE_POINTS).fill(IDNA_DISALLOWED)
const idnaMappings = new Map()

for (const [codes, status, mapping] of lines('IdnaMappingTable.txt')) {
  const [start, end] = range(codes)

  let value

  switch (status) {
    case 'valid':
    case 'deviation':
      value = IDNA_VALID
      break
    case 'ignored':
      value = IDNA_IGNORED
      break
    case 'mapped':
      value = IDNA_MAPPED
      break
    case 'disallowed':
      value = IDNA_DISALLOWED
      break
    default:
      throw new Error(`Unknown IDNA status: ${status}`)
  }

  for (let c = start; c <= end; c++) {
    idnaStatuses[c] = value

    // Every code point of a mapped range shares the same mapping.
    if (value === IDNA_MAPPED) idnaMappings.set(c, codePoints(mapping))
  }
}

/**
 * UnicodeData.txt
 *
 * https://www.unicode.org/reports/tr44/#UnicodeData.txt
 */

const combiningClasses = new Uint8Array(CODE_POINTS)
const decompositions = new Map()

{
  let first = null

  for (const fields of lines('UnicodeData.txt')) {
    const c = parseInt(fields[0], 16)
    const name = fields[1]
    const combiningClass = parseInt(fields[3], 10)
    const decomposition = fields[5]

    // Large blocks of code points sharing the same properties are given as a
    // pair of `First` and `Last` lines rather than one line per code point.
    if (name.endsWith(', First>')) {
      first = c
      continue
    }

    let start = c

    if (name.endsWith(', Last>')) {
      start = first
      first = null
    }

    for (let i = start; i <= c; i++) combiningClasses[i] = combiningClass

    // Only canonical decompositions take part in normalization; compatibility
    // decompositions are tagged with a bracketed formatting tag.
    if (decomposition !== '' && !decomposition.startsWith('<')) {
      decompositions.set(c, codePoints(decomposition))
    }
  }
}

/**
 * CompositionExclusions.txt
 *
 * https://www.unicode.org/reports/tr44/#CompositionExclusions.txt
 */

const exclusions = new Set()

for (const [codes] of lines('CompositionExclusions.txt')) {
  const [start, end] = range(codes)

  for (let c = start; c <= end; c++) exclusions.add(c)
}

/**
 * DerivedBidiClass.txt, DerivedJoiningType.txt, DerivedGeneralCategory.txt
 *
 * https://www.unicode.org/reports/tr44/#Extracted_Properties
 */

function property(file, values, fallback) {
  const property = new Uint8Array(CODE_POINTS).fill(fallback)

  // The `@missing` annotations are ordered from least to most specific, with
  // later ones overriding earlier ones.
  for (const [codes, value] of missing(file)) {
    if (!(value in values)) continue

    const [start, end] = range(codes)

    property.fill(values[value], start, end + 1)
  }

  for (const [codes, value] of lines(file)) {
    const [start, end] = range(codes)

    property.fill(value in values ? values[value] : fallback, start, end + 1)
  }

  return property
}

const bidiClasses = property('DerivedBidiClass.txt', BIDI_CLASSES, 0)

const joiningTypes = property('DerivedJoiningType.txt', JOINING_TYPES, 0)

const marks = property('DerivedGeneralCategory.txt', { Mn: 1, Mc: 1, Me: 1 }, 0)

/**
 * Table construction
 */

// Compresses a per code point property array into a sorted, gap-free list of
// ranges, each packed as `(start << 8) | value`.
function ranges(property) {
  const ranges = []

  for (let c = 0; c < CODE_POINTS; c++) {
    if (c === 0 || property[c] !== property[c - 1]) {
      ranges.push(c * 0x100 + property[c])
    }
  }

  return ranges
}

const propertyRanges = ranges(
  Uint8Array.from({ length: CODE_POINTS }, (_, c) => {
    return bidiClasses[c] | (joiningTypes[c] << 4) | (marks[c] << 7)
  })
)

const combiningClassRanges = ranges(combiningClasses)

// The number of low bits of a code point that a block index leaves to the search
// it narrows down.
const BLOCK_SHIFT = 9
const BLOCKS = (CODE_POINTS >> BLOCK_SHIFT) + 1

// Indexes a sorted, gap-free list of ranges by block, each entry giving the
// range that covers the first code point of the block. The range covering any
// code point of a block therefore lies between the entries of that block and the
// one after it, which is a far narrower search than the whole table.
function rangeBlocks(starts) {
  const blocks = []

  for (let b = 0, i = 0; b < BLOCKS; b++) {
    const start = b << BLOCK_SHIFT

    while (i + 1 < starts.length && starts[i + 1] <= start) i++

    blocks.push(i)
  }

  return blocks
}

// Indexes a list of entries sorted by code point, each entry giving the first
// entry of the block, so that a block holds the entries from its own index up to
// that of the block after it.
function entryBlocks(codePoints) {
  const blocks = []

  for (let b = 0, i = 0; b < BLOCKS; b++) {
    const start = b << BLOCK_SHIFT

    while (i < codePoints.length && codePoints[i] < start) i++

    blocks.push(i)
  }

  return blocks
}

// A pool of code point sequences that entries of the mapping and decomposition
// tables refer to by offset and length. Identical sequences are shared.
function pool() {
  const data = []
  const offsets = new Map()

  return {
    data,
    add(sequence) {
      const key = sequence.join(' ')

      let offset = offsets.get(key)

      if (offset === undefined) {
        offset = data.length

        offsets.set(key, offset)

        data.push(...sequence)
      }

      return offset
    }
  }
}

const idnaMappingData = pool()
const idnaRanges = []

for (let c = 0; c < CODE_POINTS; c++) {
  const status = idnaStatuses[c]
  const mapping = idnaMappings.get(c)

  if (c > 0) {
    const previous = idnaMappings.get(c - 1)

    // A mapped range may only be extended by a code point sharing its mapping.
    if (
      status === idnaStatuses[c - 1] &&
      (mapping === undefined || mapping.join(' ') === previous.join(' '))
    ) {
      continue
    }
  }

  let value = status

  if (mapping !== undefined) {
    if (mapping.length >= 0x20) {
      throw new Error(`Mapping of ${c.toString(16)} is too long`)
    }

    value += mapping.length * 0x4
    value += idnaMappingData.add(mapping) * 0x80
  }

  idnaRanges.push([c, value])
}

const decompositionData = pool()
const decompositionEntries = []

// Decompositions are recursive, so are expanded up front to leave the runtime
// with a single lookup per code point.
function decompose(c) {
  const decomposition = decompositions.get(c)

  if (decomposition === undefined) return [c]

  return decomposition.flatMap(decompose)
}

for (const c of [...decompositions.keys()].sort((a, b) => a - b)) {
  const decomposition = decompose(c)

  if (decomposition.length >= 0x100) {
    throw new Error(`Decomposition of ${c.toString(16)} is too long`)
  }

  const offset = decompositionData.add(decomposition)

  decompositionEntries.push([c, decomposition.length * 0x1000000 + offset])
}

// The pairs of code points that compose into a primary composite. Singleton
// decompositions, decompositions that begin with a non-starter, and the code
// points listed in CompositionExclusions.txt are all excluded.
const compositions = []

for (const [c, decomposition] of decompositions) {
  if (decomposition.length !== 2) continue
  if (combiningClasses[decomposition[0]] !== 0) continue
  if (exclusions.has(c)) continue

  compositions.push([decomposition[0], decomposition[1], c])
}

compositions.sort((a, b) => a[0] - b[0] || a[1] - b[1])

if (decompositionData.data.length >= 0x1000000) {
  throw new Error('Decomposition data is too large')
}

if (idnaMappingData.data.length >= 0x2000000) {
  throw new Error('Mapping data is too large')
}

// Every domain is made up largely of ASCII code points, which are worth looking
// up by code point rather than by searching for the range that holds them.
const idnaAscii = []

for (let c = 0, i = 0; c < 0x80; c++) {
  while (i + 1 < idnaRanges.length && idnaRanges[i + 1][0] <= c) i++

  idnaAscii.push(idnaRanges[i][1])
}

const propertyBlocks = rangeBlocks(propertyRanges.map((range) => range >>> 8))

const combiningClassBlocks = rangeBlocks(combiningClassRanges.map((range) => range >>> 8))

const idnaBlocks = rangeBlocks(idnaRanges.map(([c]) => c))

const decompositionBlocks = entryBlocks(decompositionEntries.map(([c]) => c))

for (const [name, table] of [
  ['property', propertyRanges],
  ['combining class', combiningClassRanges],
  ['IDNA', idnaRanges],
  ['decomposition', decompositionEntries]
]) {
  if (table.length > 0xffff) {
    throw new Error(`The ${name} table is too large to index by block`)
  }
}

/**
 * Output
 */

function hex(value) {
  return `0x${value.toString(16)}`
}

// Wraps a list of already formatted entries across as many lines as needed.
function table(entries, perLine) {
  let out = ''

  for (let i = 0; i < entries.length; i += perLine) {
    out += `  ${entries.slice(i, i + perLine).join(', ')},\n`
  }

  return out
}

const banner = `// This file was generated from the Unicode Character Database ${version} by
// scripts/generate-unicode-tables.mjs. Do not edit.
`

fs.writeFileSync(
  header,
  `${banner}
#ifndef URL_UNICODE_TABLES_H
#define URL_UNICODE_TABLES_H

#include <stdint.h>
#include <utf.h>

#define URL_UNICODE_VERSION "${version}"

/**
 * A range of code points sharing the same property value, packed as
 * \`(start << 8) | value\`. Ranges are sorted and gap-free, each one extending
 * up to the start of the range that follows it.
 */
typedef uint32_t url_unicode_range_t;

/**
 * A block index narrows a search down to the entries that may hold a given code
 * point. The high \`21 - URL_UNICODE_BLOCK_SHIFT\` bits of the code point index
 * the block, and the entries of interest are those from the block's own index up
 * to and including that of the block after it.
 */
#define URL_UNICODE_BLOCK_SHIFT ${BLOCK_SHIFT}

#define URL_UNICODE_BLOCKS ${BLOCKS}

/**
 * The Bidi_Class values referenced by the Bidi rule of RFC 5893. Code points of
 * any other class are given \`url__unicode_bidi_class_other\`, which no label of
 * a Bidi domain name may contain.
 *
 * https://www.rfc-editor.org/rfc/rfc5893#section-2
 */
typedef enum {
  url__unicode_bidi_class_other = 0,
  url__unicode_bidi_class_l = ${BIDI_CLASSES.L},
  url__unicode_bidi_class_r = ${BIDI_CLASSES.R},
  url__unicode_bidi_class_al = ${BIDI_CLASSES.AL},
  url__unicode_bidi_class_an = ${BIDI_CLASSES.AN},
  url__unicode_bidi_class_en = ${BIDI_CLASSES.EN},
  url__unicode_bidi_class_es = ${BIDI_CLASSES.ES},
  url__unicode_bidi_class_cs = ${BIDI_CLASSES.CS},
  url__unicode_bidi_class_et = ${BIDI_CLASSES.ET},
  url__unicode_bidi_class_on = ${BIDI_CLASSES.ON},
  url__unicode_bidi_class_bn = ${BIDI_CLASSES.BN},
  url__unicode_bidi_class_nsm = ${BIDI_CLASSES.NSM},
} url__unicode_bidi_class_t;

/**
 * The Joining_Type values referenced by the ContextJ rules of RFC 5892.
 *
 * https://www.rfc-editor.org/rfc/rfc5892#appendix-A
 */
typedef enum {
  url__unicode_joining_type_other = 0,
  url__unicode_joining_type_left = ${JOINING_TYPES.L},
  url__unicode_joining_type_right = ${JOINING_TYPES.R},
  url__unicode_joining_type_dual = ${JOINING_TYPES.D},
  url__unicode_joining_type_transparent = ${JOINING_TYPES.T},
} url__unicode_joining_type_t;

/**
 * The Bidi_Class, Joining_Type, and General_Category=Mark properties of a code
 * point, packed into the value of a range as
 * \`bidi_class | (joining_type << 4) | (mark << 7)\`.
 */
extern const url_unicode_range_t url__unicode_property_ranges[${propertyRanges.length}];

extern const uint16_t url__unicode_property_blocks[URL_UNICODE_BLOCKS];

/**
 * The Canonical_Combining_Class property of a code point.
 */
extern const url_unicode_range_t url__unicode_combining_class_ranges[${combiningClassRanges.length}];

extern const uint16_t url__unicode_combining_class_blocks[URL_UNICODE_BLOCKS];

/**
 * The full canonical decomposition of a code point, given as a length and an
 * offset into \`url__unicode_decomposition_data\` packed as
 * \`(length << 24) | offset\`. Entries are sorted by code point.
 */
typedef struct {
  utf32_t code_point;
  uint32_t decomposition;
} url__unicode_decomposition_t;

extern const url__unicode_decomposition_t url__unicode_decompositions[${decompositionEntries.length}];

extern const uint16_t url__unicode_decomposition_blocks[URL_UNICODE_BLOCKS];

extern const utf32_t url__unicode_decomposition_data[${decompositionData.data.length}];

/**
 * A pair of code points and the primary composite they compose into. Entries
 * are sorted by first and then second code point.
 */
typedef struct {
  utf32_t first;
  utf32_t second;
  utf32_t composite;
} url__unicode_composition_t;

extern const url__unicode_composition_t url__unicode_compositions[${compositions.length}];

/**
 * The status values of the IDNA mapping table. Deviation is folded into valid
 * as the domain parser only ever performs nontransitional processing, which
 * leaves deviation code points unchanged and accepts them as valid.
 *
 * https://www.unicode.org/reports/tr46/#Table_Data_File_Fields
 */
typedef enum {
  url__idna_status_valid = ${IDNA_VALID},
  url__idna_status_ignored = ${IDNA_IGNORED},
  url__idna_status_mapped = ${IDNA_MAPPED},
  url__idna_status_disallowed = ${IDNA_DISALLOWED},
} url__idna_status_t;

/**
 * A range of code points sharing the same IDNA status and, for mapped ranges,
 * the same mapping. The mapping is given as a length and an offset into
 * \`url__idna_mapping_data\`, packed together with the status as
 * \`status | (length << 2) | (offset << 7)\`. Ranges are sorted and gap-free,
 * each one extending up to the start of the range that follows it.
 */
typedef struct {
  utf32_t start;
  uint32_t mapping;
} url_idna_range_t;

extern const url_idna_range_t url__idna_ranges[${idnaRanges.length}];

extern const uint16_t url__idna_blocks[URL_UNICODE_BLOCKS];

/**
 * The mapping of every ASCII code point, indexed by the code point itself.
 */
extern const uint32_t url__idna_ascii[0x80];

extern const utf32_t url__idna_mapping_data[${idnaMappingData.data.length}];

#endif // URL_UNICODE_TABLES_H
`
)

fs.writeFileSync(
  source,
  `${banner}
#include <stdint.h>
#include <url/unicode-tables.h>
#include <utf.h>

const url_unicode_range_t url__unicode_property_ranges[${propertyRanges.length}] = {
${table(propertyRanges.map(hex), 8)}};

const uint16_t url__unicode_property_blocks[URL_UNICODE_BLOCKS] = {
${table(propertyBlocks.map(String), 12)}};

const url_unicode_range_t url__unicode_combining_class_ranges[${combiningClassRanges.length}] = {
${table(combiningClassRanges.map(hex), 8)}};

const uint16_t url__unicode_combining_class_blocks[URL_UNICODE_BLOCKS] = {
${table(combiningClassBlocks.map(String), 12)}};

const url__unicode_decomposition_t url__unicode_decompositions[${decompositionEntries.length}] = {
${table(
  decompositionEntries.map(([c, d]) => `{${hex(c)}, ${hex(d)}}`),
  4
)}};

const uint16_t url__unicode_decomposition_blocks[URL_UNICODE_BLOCKS] = {
${table(decompositionBlocks.map(String), 12)}};

const utf32_t url__unicode_decomposition_data[${decompositionData.data.length}] = {
${table(decompositionData.data.map(hex), 8)}};

const url__unicode_composition_t url__unicode_compositions[${compositions.length}] = {
${table(
  compositions.map(([a, b, c]) => `{${hex(a)}, ${hex(b)}, ${hex(c)}}`),
  4
)}};

const url_idna_range_t url__idna_ranges[${idnaRanges.length}] = {
${table(
  idnaRanges.map(([c, v]) => `{${hex(c)}, ${hex(v)}}`),
  4
)}};

const uint16_t url__idna_blocks[URL_UNICODE_BLOCKS] = {
${table(idnaBlocks.map(String), 12)}};

const uint32_t url__idna_ascii[0x80] = {
${table(idnaAscii.map(hex), 8)}};

const utf32_t url__idna_mapping_data[${idnaMappingData.data.length}] = {
${table(idnaMappingData.data.map(hex), 8)}};
`
)
