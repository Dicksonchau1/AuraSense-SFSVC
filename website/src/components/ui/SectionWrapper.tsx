import type { ReactNode } from 'react';

interface SectionWrapperProps {
  id?: string;
  className?: string;
  children: ReactNode;
  dark?: boolean;
  compact?: boolean;
}

export function SectionWrapper({
  id,
  className = '',
  children,
  dark = false,
  compact = false,
}: SectionWrapperProps) {
  return (
    <section
      id={id}
      style={{ backgroundColor: dark ? 'var(--color-surface-raised)' : 'var(--color-surface)' }}
      className={className}
    >
      <div
        style={{
          maxWidth: '1200px',
          margin: '0 auto',
          padding: compact ? '3rem 1.5rem' : '5rem 1.5rem',
        }}
      >
        {children}
      </div>
    </section>
  );
}
