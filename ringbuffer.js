export default class RingBuffer {
  constructor(size) {
    this.size = size;
    this.buffer = new Array(size);
    this.index = 0;
    this.length = 0;
  }

  push(item) {
    this.buffer[this.index] = item;
    this.index = (this.index + 1) % this.size;
    if (this.length < this.size) this.length++;
  }

  toArray() {
    const result = [];
    for (let i = 0; i < this.length; i++) {
      const idx = (this.index - this.length + i + this.size) % this.size;
      result.push(this.buffer[idx]);
    }
    return result;
  }
}