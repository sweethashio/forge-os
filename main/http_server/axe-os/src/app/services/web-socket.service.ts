import { Injectable, OnDestroy } from '@angular/core';
import { Observable, Subject, timer } from 'rxjs';
import { webSocket, WebSocketSubject } from 'rxjs/webSocket';
import { retryWhen, tap, delayWhen, takeUntil } from 'rxjs/operators';

const RECONNECT_INTERVAL_MS = 5000;

@Injectable({
  providedIn: 'root'
})
export class WebsocketService implements OnDestroy {

  private socket$!: WebSocketSubject<string>;
  private destroy$ = new Subject<void>();

  /** Observable that emits messages and auto-reconnects on disconnect. */
  public ws$: Observable<string>;

  constructor() {
    this.socket$ = this.createSocket();
    this.ws$ = this.socket$.pipe(
      retryWhen(errors =>
        errors.pipe(
          tap(() => console.warn(`WebSocket disconnected. Reconnecting in ${RECONNECT_INTERVAL_MS}ms…`)),
          delayWhen(() => timer(RECONNECT_INTERVAL_MS))
        )
      ),
      takeUntil(this.destroy$)
    );
  }

  private createSocket(): WebSocketSubject<string> {
    return webSocket<string>({
      url: `ws://${window.location.host}/api/ws`,
      deserializer: (e: MessageEvent) => e.data as string,
    });
  }

  ngOnDestroy(): void {
    this.destroy$.next();
    this.destroy$.complete();
    this.socket$.complete();
  }
}