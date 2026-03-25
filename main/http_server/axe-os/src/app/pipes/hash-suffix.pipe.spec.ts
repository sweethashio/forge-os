import { HashSuffixPipe } from './hash-suffix.pipe';

describe('HashSuffixPipe', () => {
  let pipe: HashSuffixPipe;

  beforeEach(() => {
    pipe = new HashSuffixPipe();
  });

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('returns N/A for null', () => {
    expect(pipe.transform(null as any)).toBe('N/A');
  });

  it('returns N/A for undefined', () => {
    expect(pipe.transform(undefined as any)).toBe('N/A');
  });

  it('returns N/A for NaN', () => {
    expect(pipe.transform(NaN)).toBe('N/A');
  });

  it('returns N/A for Infinity', () => {
    expect(pipe.transform(Infinity)).toBe('N/A');
  });

  it('returns N/A for -Infinity', () => {
    expect(pipe.transform(-Infinity)).toBe('N/A');
  });

  it('returns N/A for negative values', () => {
    expect(pipe.transform(-1)).toBe('N/A');
  });

  it('returns 0.00 H/s for zero', () => {
    expect(pipe.transform(0)).toBe('0.00 H/s');
  });

  it('formats H/s correctly', () => {
    expect(pipe.transform(500)).toBe('500 H/s');
  });

  it('formats KH/s correctly', () => {
    expect(pipe.transform(1500)).toBe('1.50 KH/s');
  });

  it('formats MH/s correctly', () => {
    expect(pipe.transform(1500000)).toBe('1.50 MH/s');
  });

  it('formats GH/s correctly', () => {
    expect(pipe.transform(1500000000)).toBe('1.50 GH/s');
  });

  it('formats TH/s correctly', () => {
    expect(pipe.transform(2475000000000)).toBe('2.48 TH/s');
  });

  it('does not produce undefined suffix for astronomically large values', () => {
    const result = pipe.transform(1e30);
    expect(result).not.toContain('undefined');
  });

  it('static transform matches instance transform', () => {
    expect(HashSuffixPipe.transform(1000)).toBe(pipe.transform(1000));
  });
});
