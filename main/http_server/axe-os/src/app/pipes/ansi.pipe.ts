import { Pipe, PipeTransform } from '@angular/core';

// Matches ANSI/VT100 escape sequences: ESC [ ... m  and other control sequences
const ANSI_ESCAPE_RE = /\x1b\[[0-9;]*[A-Za-z]/g;

@Pipe({
  name: 'ANSI',
  pure: true
})
export class ANSIPipe implements PipeTransform {

  transform(value: string): string {
    if (value == null) {
      return '';
    }
    return value.replace(ANSI_ESCAPE_RE, '');
  }

}
