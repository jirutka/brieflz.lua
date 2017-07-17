import from_base64 from require 'basexx'
import pack, depack from require 'brieflz'

data = 'Lórem ipsum dolor šít amet, ipsum amet dolor'
data_packed = from_base64 'TAAAw7NyZW0gaXBzdW0gZG9sbwUAciDFocOtdCBhbWV0LFCIFwsc'


describe 'pack', ->

  context 'given empty string', ->
    it 'returns empty string', ->
      packed, orig_size, packed_size = pack('')

      assert.same '', packed
      assert.same 0, orig_size
      assert.same 0, packed_size

  context 'given non-empty string', ->
    it 'returns packed data', ->
      actual, orig_size, packed_size = pack(data)

      assert.same data_packed, actual
      assert.same #data, orig_size
      assert.same #actual, packed_size

  context 'given nil', ->
    it 'raises bad argument error', ->
      assert.has_error -> pack(nil)


describe 'depack', ->

  context 'given empty data, 0', ->
    it 'returns empty string', ->
      assert.same '', depack('', 0)

  context 'given valid data', ->

    context 'and valid size', ->
      it 'returns unpacked data', ->
        assert.same data, depack(data_packed, #data)

    context 'and invalid size', ->
      it 'raises error', ->
        for i in *{3, -3}
          assert.has_error -> depack data_packed, #data + i

    context 'and negative size', ->
      it 'raises error', ->
        assert.has_error -> depack data_packed, -3

  context 'given invalid data', ->
    it 'raises error', ->
      assert.has_error -> depack(data, #data_packed)
      assert.has_error -> depack(data, #data_packed - 3)

  context 'given nil, 0', ->
    it 'raises error', ->
      assert.has_error -> depack nil, 0
