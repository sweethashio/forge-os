import { DiffSuffixPipe } from './diff-suffix.pipe';

describe('DiffSuffixPipe', () => {
  let pipe: DiffSuffixPipe;

  beforeEach(() => {
    pipe = new DiffSuffixPipe();
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

  it('returns N/A for negative values', () => {
    expect(pipe.transform(-1)).toBe('N/A');
  });

  it('returns 0 for zero', () => {
    expect(pipe.transform(0)).toBe('0');
  });

  it('formats plain difficulty correctly', () => {
    expect(pipe.transform(500)).toBe('500');
  });

  it('formats K suffix correctly', () => {
    expect(pipe.transform(1500)).toBe('1.50 K');
  });

  it('formats M suffix correctly', () => {
    expect(pipe.transform(1500000)).toBe('1.50 M');
  });

  it('formats G suffix correctly', () => {
    expect(pipe.transform(1500000000)).toBe('1.50 G');
  });

  it('does not produce undefined suffix for very large values', () => {
    const result = pipe.transform(1e30);
    expect(result).not.toContain('undefined');
  });

  it('static transform matches instance transform', () => {
    expect(DiffSuffixPipe.transform(1000)).toBe(pipe.transform(1000));
  });
});