import { isAbortError } from "common/requestQueue";
import { ButtonCard, Card } from "components/Card";
import { ToastMessage } from "components/ToastMessage";
import { AppPage } from "pages/AppPage";
import { type JSX } from "preact";
import { useEffect, useState } from "preact/hooks";
import { PageContents } from "../PageContents";
import { PageHeading } from "../PageHeading";

interface UIButtonInfo {
  name: string;
  title: string;
  mustConfirm: boolean;
}

export function ControlPage(): JSX.Element {
  return (
    <AppPage>
      <PageHeading title="Control" />
      <PageContents>
        <ControlCards />
      </PageContents>
    </AppPage>
  );
}

function ControlCards(): JSX.Element {
  const [buttons, setButtons] = useState<UIButtonInfo[] | null>(null);
  const [failed, setFailed] = useState<boolean>(false);
  const [reloadToken, setReloadToken] = useState<number>(0);

  useEffect(() => {
    const controller = new AbortController();
    setFailed(false);

    void (async () => {
      try {
        const res = await fetch("/api/buttons", { signal: controller.signal });
        if (!res.ok) {
          throw new Error(`${res.status} ${res.statusText}`);
        }
        setButtons((await res.json()) as UIButtonInfo[]);
      } catch (e) {
        if (isAbortError(e)) return;
        console.warn("Failed to load the button list", e);
        setFailed(true);
      }
    })();

    return () => controller.abort();
  }, [reloadToken]);

  if (failed) {
    return (
      <div className="alert alert-danger" role="alert">
        <p className="mb-1">Couldn't reach the device to load its controls.</p>
        <p className="mb-2 small">
          Make sure it's powered on and connected to the network, then try
          again.
        </p>
        <button
          className="btn btn-outline-secondary btn-sm"
          type="button"
          onClick={() => setReloadToken((token) => token + 1)}
        >
          Retry
        </button>
      </div>
    );
  }

  if (buttons === null) {
    return (
      <Card loading title="">
        <></>
      </Card>
    );
  }

  if (buttons.length === 0) {
    return (
      <p>No controls have been registered by this device&apos;s firmware.</p>
    );
  }

  return (
    <div className="vstack gap-4">
      {buttons.map((button) => (
        <UIButtonCard key={button.name} button={button} />
      ))}
    </div>
  );
}

interface UIButtonCardProps {
  button: UIButtonInfo;
}

function UIButtonCard({ button }: UIButtonCardProps): JSX.Element {
  const [httpErrorText, setHttpErrorText] = useState("");
  const [showSuccessToast, setShowSuccessToast] = useState(false);
  const [pending, setPending] = useState(false);

  async function handleClick(): Promise<void> {
    if (
      button.mustConfirm &&
      !window.confirm(`Are you sure you want to trigger "${button.title}"?`)
    ) {
      return;
    }

    setPending(true);
    try {
      const response = await fetch(
        `/api/buttons/${encodeURIComponent(button.name)}`,
        { method: "POST" },
      );
      if (!response.ok) {
        setHttpErrorText(`${response.status} ${response.statusText}`);
        return;
      }
      setShowSuccessToast(true);
    } catch (e) {
      setHttpErrorText(String(e));
    } finally {
      setPending(false);
    }
  }

  return (
    <>
      <ToastMessage
        color="text-bg-danger"
        show={httpErrorText !== ""}
        onHide={() => setHttpErrorText("")}
      >
        <p>Failed to trigger &quot;{button.title}&quot;:</p>
        <p>{httpErrorText}</p>
      </ToastMessage>
      <ToastMessage
        color="text-bg-success"
        show={showSuccessToast}
        onHide={() => setShowSuccessToast(false)}
      >
        <p>Triggered &quot;{button.title}&quot;.</p>
      </ToastMessage>
      <ButtonCard
        title={button.title}
        buttonText={button.title}
        buttonVariant={button.mustConfirm ? "danger" : "primary"}
        isButtonEnabled={!pending}
        onClick={() => {
          void handleClick();
        }}
      >
        <></>
      </ButtonCard>
    </>
  );
}
