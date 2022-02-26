import Page from './index';
let offscreenCanvas;

export class OffscreenCanvas {
  private _bridge: OffscreenCanvasBridge;
  private _width: number;
  private _height: number;
  constructor(page: Page, width: number, height: number) {
    offscreenCanvas = page.requireModule('system.offscreenCanvas');
    this._height = height;
    this._width = width;
    this._bridge = offscreenCanvas.create({
      width: this._width,
      height: this._height
    });
  }
  public getContext(contextId: '2d', options?: CanvasRenderingContext2DSettings): OffscreenCanvasRenderingContext2D {
    return this._bridge.getContext('2d');
  }

  public transferToImageBitmap(): ImageBitmap {
    return this._bridge.transferToImageBitmap();
  }

  public toDataURL(type?: string, quality?: number): string {
    return this._bridge.toDataURL(type, quality);
  }

  public get width() {
    return this._width;
  }

  public set width(width) {
    this._width = width;
  }

  public get height() {
    return this._height;
  }

  public set height(height) {
    this._height = height;
  }
}

export interface OffscreenCanvasRenderingContext2D {
}

export interface OffscreenCanvasBridge {
  getContext: (contextId: '2d', options?: CanvasRenderingContext2DSettings)=>OffscreenCanvasRenderingContext2D;
  transferToImageBitmap: ()=>ImageBitmap;
  toDataURL:(type?: string, quality?: number)=>string;
}

export interface CanvasRenderingContext2DSettings {
  alpha?: boolean;
  desynchronized?: boolean;
}

export class ImageBitmap {
  readonly height: number;
  readonly width: number;
  private _bridgeId: number;
  constructor(bridgeId: number) {
    this._bridgeId = bridgeId;
  }

  public get bridgeId() {
    return this._bridgeId;
  }

  public set bridgeId(bridgeId) {
    this._bridgeId = bridgeId;
  }
}
