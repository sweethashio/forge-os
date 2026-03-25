import { ANSIPipe } from './ansi.pipe';

describe('ANSIPipe', () => {
  let pipe: ANSIPipe;

  beforeEach(() => {
    pipe = new ANSIPipe();
  });

  it('create an instance', () => {
    expect(pipe).toBeTruthy();
  });

  it('returns empty string for null', () => {
    expect(pipe.transform(null as any)).toBe('');
  });

  it('returns empty string for undefined', () => {
    expect(pipe.transform(undefined as any)).toBe('');
  });

  it('returns plain text unchanged', () => {
    expect(pipe.transform('hello world')).toBe('hello world');
  });

  it('strips a single color code', () => {
    expect(pipe.transform('\x1b[32mgreen\x1b[0m')).toBe('green');
  });

  it('strips multiple ANSI codes', () => {
    expect(pipe.transform('\x1b[1;33mWARN\x1b[0m: message')).toBe('WARN: message');
  });

  it('strips complex escape sequences', () => {
    expect(pipe.transform('\x1b[0;31mERROR\x1b[0m text')).toBe('ERROR text');
  });

  it('handles string with no escape sequences', () => {
    expect(pipe.transform('no escapes here')).toBe('no escapes here');
  });
});
